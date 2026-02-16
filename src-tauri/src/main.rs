#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

use serde::{Deserialize, Serialize};
use std::{
    collections::{BTreeMap, BTreeSet, HashMap},
    fs,
    io::Cursor,
    path::{Path, PathBuf},
    ptr,
    process::Command,
    sync::{
        atomic::{AtomicU64, Ordering},
        Mutex,
    },
    time::{SystemTime, UNIX_EPOCH},
};
use tauri::{
    menu::{Menu, MenuItem},
    tray::{MouseButton, MouseButtonState, TrayIcon, TrayIconBuilder, TrayIconEvent},
    AppHandle, Emitter, Manager, WebviewUrl, WebviewWindowBuilder, Window, WindowEvent,
};
use tauri_plugin_global_shortcut::{GlobalShortcutExt, ShortcutState};

static ID_COUNTER: AtomicU64 = AtomicU64::new(1);

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct LauncherData {
    version: u8,
    groups: Vec<Group>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Group {
    id: String,
    name: String,
    order: i32,
    items: Vec<LaunchItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct LaunchItem {
    id: String,
    item_type: String,
    name: String,
    target_path: String,
    icon_location: String,
    arguments: String,
    launch_count: u64,
    enabled: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Settings {
    hotkey: String,
    execute_hide: bool,
    current_group: Option<String>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
struct LauncherState {
    groups: Vec<Group>,
    settings: Settings,
    item_icons: BTreeMap<String, String>,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(rename_all = "camelCase")]
struct ItemInput {
    id: Option<String>,
    item_type: Option<String>,
    name: String,
    target_path: String,
    icon_location: String,
    arguments: String,
    enabled: Option<bool>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
struct LaunchResult {
    ok: bool,
    message: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
struct EditorContext {
    group_id: String,
    item: Option<LaunchItem>,
}

#[derive(Debug, Clone, Deserialize)]
struct LegacyItem {
    #[serde(rename = "Name")]
    name: String,
    #[serde(rename = "TargetPath")]
    target_path: String,
    #[serde(rename = "IconLocation")]
    icon_location: String,
    #[serde(rename = "Arguments")]
    arguments: String,
    #[serde(rename = "Count")]
    count: Option<u64>,
}

struct AppState {
    data: Mutex<LauncherData>,
    settings: Mutex<Settings>,
    editor_context: Mutex<Option<EditorContext>>,
    tray_icon: Mutex<Option<TrayIcon>>,
    icon_cache: Mutex<HashMap<String, String>>,
    icons_dir: PathBuf,
    icons_index_path: PathBuf,
    data_path: PathBuf,
    settings_path: PathBuf,
}

impl AppState {
    fn save_data(&self) -> Result<(), String> {
        let data = self.data.lock().map_err(|_| "state lock poisoned".to_string())?;
        write_json_atomic(&self.data_path, &*data)
    }

    fn save_settings(&self) -> Result<(), String> {
        let settings = self
            .settings
            .lock()
            .map_err(|_| "settings lock poisoned".to_string())?;
        write_json_atomic(&self.settings_path, &*settings)
    }
}

fn generate_id(prefix: &str) -> String {
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_millis())
        .unwrap_or(0);
    let idx = ID_COUNTER.fetch_add(1, Ordering::Relaxed);
    format!("{prefix}_{now}_{idx}")
}

fn is_separator_item(name: &str, target: &str, icon: &str) -> bool {
    let trimmed = name.trim();
    (target.trim().is_empty() && icon.trim().is_empty())
        || (trimmed.starts_with("----") && trimmed.ends_with("----"))
}

fn parse_bool_flag(value: Option<&String>, default: bool) -> bool {
    match value.map(|v| v.trim()) {
        Some("1") => true,
        Some("0") => false,
        Some(v) if v.eq_ignore_ascii_case("true") => true,
        Some(v) if v.eq_ignore_ascii_case("false") => false,
        _ => default,
    }
}

fn parse_poner_cfg(path: &Path) -> Settings {
    let mut map = BTreeMap::<String, String>::new();
    if let Ok(content) = fs::read_to_string(path) {
        for line in content.lines() {
            let line = line.trim();
            if line.starts_with('[') || line.is_empty() || !line.contains('=') {
                continue;
            }
            let mut parts = line.splitn(2, '=');
            if let (Some(k), Some(v)) = (parts.next(), parts.next()) {
                map.insert(k.trim().to_string(), v.trim().to_string());
            }
        }
    }

    let hotkey = "Alt+1".to_string();
    let execute_hide = parse_bool_flag(map.get("ExecuteHide"), true);
    let current_group = map
        .get("CurrentTab")
        .or_else(|| map.get("ActivatedTab"))
        .cloned();

    Settings {
        hotkey,
        execute_hide,
        current_group,
    }
}

fn app_base_dir() -> PathBuf {
    if let Some(dir) = dirs::data_local_dir() {
        let base = dir.join("nassistant");
        let _ = fs::create_dir_all(&base);
        return base;
    }
    let fallback = std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."));
    let _ = fs::create_dir_all(&fallback);
    fallback
}

fn find_legacy_file(file_name: &str) -> Option<PathBuf> {
    let mut candidates = Vec::new();
    if let Ok(dir) = std::env::current_dir() {
        candidates.push(dir.join(file_name));
    }
    if let Ok(exe) = std::env::current_exe() {
        if let Some(parent) = exe.parent() {
            candidates.push(parent.join(file_name));
            if let Some(grand) = parent.parent() {
                candidates.push(grand.join(file_name));
            }
        }
    }

    candidates.into_iter().find(|p| p.exists())
}

fn default_launcher_data() -> LauncherData {
    LauncherData {
        version: 2,
        groups: vec![Group {
            id: generate_id("group"),
            name: "Common".to_string(),
            order: 0,
            items: Vec::new(),
        }],
    }
}

fn convert_legacy_data(raw: &str) -> Result<LauncherData, String> {
    let parsed: BTreeMap<String, Vec<LegacyItem>> =
        serde_json::from_str(raw).map_err(|e| format!("parse legacy Data.json failed: {e}"))?;

    let mut groups = Vec::new();
    for (idx, (name, items)) in parsed.into_iter().enumerate() {
        let mut mapped_items = Vec::new();
        for item in items {
            let item_type = if is_separator_item(&item.name, &item.target_path, &item.icon_location) {
                "separator"
            } else {
                "app"
            };
            mapped_items.push(LaunchItem {
                id: generate_id("item"),
                item_type: item_type.to_string(),
                name: item.name,
                target_path: item.target_path,
                icon_location: item.icon_location,
                arguments: item.arguments,
                launch_count: item.count.unwrap_or(0),
                enabled: true,
            });
        }

        groups.push(Group {
            id: generate_id("group"),
            name,
            order: idx as i32,
            items: mapped_items,
        });
    }

    if groups.is_empty() {
        groups.push(Group {
            id: generate_id("group"),
            name: "Common".to_string(),
            order: 0,
            items: Vec::new(),
        });
    }

    Ok(LauncherData { version: 2, groups })
}

fn write_json_atomic<T: Serialize>(path: &Path, value: &T) -> Result<(), String> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).map_err(|e| format!("create dir failed: {e}"))?;
    }
    let tmp = path.with_extension("json.tmp");
    let content = serde_json::to_string_pretty(value).map_err(|e| format!("serialize failed: {e}"))?;
    fs::write(&tmp, content).map_err(|e| format!("write tmp failed: {e}"))?;
    fs::rename(&tmp, path).map_err(|e| format!("replace file failed: {e}"))?;
    Ok(())
}

fn write_text_if_changed(path: &Path, content: &str) -> Result<(), String> {
    if let Ok(existing) = fs::read_to_string(path) {
        if existing == content {
            return Ok(());
        }
    }
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).map_err(|e| format!("create dir failed: {e}"))?;
    }
    fs::write(path, content).map_err(|e| format!("write file failed: {e}"))?;
    Ok(())
}

fn split_windows_args(arguments: &str) -> Vec<String> {
    let mut result = Vec::new();
    let mut current = String::new();
    let mut in_quotes = false;

    for ch in arguments.chars() {
        match ch {
            '"' => {
                in_quotes = !in_quotes;
            }
            ' ' | '\t' if !in_quotes => {
                if !current.is_empty() {
                    result.push(current.clone());
                    current.clear();
                }
            }
            _ => current.push(ch),
        }
    }

    if !current.is_empty() {
        result.push(current);
    }

    result
}

fn load_state() -> (LauncherData, Settings, PathBuf, PathBuf) {
    let base = app_base_dir();
    let data_path = base.join("launcher.v2.json");
    let settings_path = base.join("nassistant.settings.json");

    let launcher_data = if data_path.exists() {
        fs::read_to_string(&data_path)
            .ok()
            .and_then(|raw| serde_json::from_str::<LauncherData>(&raw).ok())
            .unwrap_or_else(default_launcher_data)
    } else if let Some(legacy_path) = find_legacy_file("Data.json") {
        let imported = fs::read_to_string(&legacy_path)
            .ok()
            .and_then(|raw| convert_legacy_data(&raw).ok())
            .unwrap_or_else(default_launcher_data);
        let _ = write_json_atomic(&data_path, &imported);
        imported
    } else {
        let initial = default_launcher_data();
        let _ = write_json_atomic(&data_path, &initial);
        initial
    };

    let settings = if settings_path.exists() {
        fs::read_to_string(&settings_path)
            .ok()
            .and_then(|raw| serde_json::from_str::<Settings>(&raw).ok())
            .unwrap_or_else(|| {
                if let Some(cfg_path) = find_legacy_file("Poner.cfg") {
                    parse_poner_cfg(&cfg_path)
                } else {
                    Settings {
                        hotkey: "Alt+1".to_string(),
                        execute_hide: true,
                        current_group: None,
                    }
                }
            })
    } else {
        let s = if let Some(cfg_path) = find_legacy_file("Poner.cfg") {
            parse_poner_cfg(&cfg_path)
        } else {
            Settings {
                hotkey: "Alt+1".to_string(),
                execute_hide: true,
                current_group: None,
            }
        };
        let _ = write_json_atomic(&settings_path, &s);
        s
    };

    (launcher_data, settings, data_path, settings_path)
}

#[tauri::command]
fn load_launcher_state(state: tauri::State<'_, AppState>) -> Result<LauncherState, String> {
    let data = state.data.lock().map_err(|_| "state lock poisoned".to_string())?;
    let settings = state
        .settings
        .lock()
        .map_err(|_| "settings lock poisoned".to_string())?;
    let item_icons = build_item_icons(&state, &data.groups);

    Ok(LauncherState {
        groups: data.groups.clone(),
        settings: settings.clone(),
        item_icons,
    })
}

#[tauri::command]
fn upsert_item(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    item: ItemInput,
) -> Result<(), String> {
    {
        let mut data = state.data.lock().map_err(|_| "state lock poisoned".to_string())?;
        let group = data
            .groups
            .iter_mut()
            .find(|g| g.id == group_id)
            .ok_or_else(|| "group not found".to_string())?;

        let item_type = item.item_type.unwrap_or_else(|| {
            if is_separator_item(&item.name, &item.target_path, &item.icon_location) {
                "separator".to_string()
            } else {
                "app".to_string()
            }
        });
        let target_path = item.target_path.trim().to_string();
        let icon_location = {
            let icon = item.icon_location.trim();
            if item_type == "app" && icon.is_empty() {
                target_path.clone()
            } else {
                icon.to_string()
            }
        };
        let arguments = item.arguments.trim().to_string();
        let name = item.name.trim().to_string();

        if let Some(id) = item.id {
            if let Some(existing) = group.items.iter_mut().find(|it| it.id == id) {
                existing.item_type = item_type;
                existing.name = name;
                existing.target_path = target_path;
                existing.icon_location = icon_location;
                existing.arguments = arguments;
                existing.enabled = item.enabled.unwrap_or(true);
            } else {
                group.items.push(LaunchItem {
                    id,
                    item_type,
                    name,
                    target_path,
                    icon_location,
                    arguments,
                    launch_count: 0,
                    enabled: item.enabled.unwrap_or(true),
                });
            }
        } else {
            group.items.push(LaunchItem {
                id: generate_id("item"),
                item_type,
                name,
                target_path,
                icon_location,
                arguments,
                launch_count: 0,
                enabled: item.enabled.unwrap_or(true),
            });
        }
    }

    state.save_data()?;
    let _ = app.emit("data-changed", "upsert_item");
    Ok(())
}

#[tauri::command]
fn delete_item(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    item_id: String,
) -> Result<(), String> {
    {
        let mut data = state.data.lock().map_err(|_| "state lock poisoned".to_string())?;
        let group = data
            .groups
            .iter_mut()
            .find(|g| g.id == group_id)
            .ok_or_else(|| "group not found".to_string())?;
        let before = group.items.len();
        group.items.retain(|it| it.id != item_id);
        if group.items.len() == before {
            return Err("item not found".to_string());
        }
    }

    state.save_data()?;
    let _ = app.emit("data-changed", "delete_item");
    Ok(())
}

#[tauri::command]
fn launch_item(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    item_id: String,
) -> Result<LaunchResult, String> {
    let (target, args, name, should_hide) = {
        let mut data = state.data.lock().map_err(|_| "state lock poisoned".to_string())?;
        let settings = state
            .settings
            .lock()
            .map_err(|_| "settings lock poisoned".to_string())?;

        let group = data
            .groups
            .iter_mut()
            .find(|g| g.id == group_id)
            .ok_or_else(|| "group not found".to_string())?;

        let item = group
            .items
            .iter_mut()
            .find(|it| it.id == item_id)
            .ok_or_else(|| "item not found".to_string())?;

        if item.item_type == "separator" {
            return Err("separator item cannot be launched".to_string());
        }
        if item.target_path.trim().is_empty() {
            return Err("target path is empty".to_string());
        }

        item.launch_count = item.launch_count.saturating_add(1);

        (
            item.target_path.clone(),
            item.arguments.clone(),
            item.name.clone(),
            settings.execute_hide,
        )
    };

    let mut command = Command::new(&target);
    let parsed_args = split_windows_args(&args);
    if !parsed_args.is_empty() {
        command.args(parsed_args);
    }

    command.spawn().map_err(|e| format!("launch failed: {e}"))?;
    state.save_data()?;

    if should_hide {
        if let Some(main) = app.get_webview_window("main") {
            let _ = main.hide();
        }
    }

    let _ = app.emit("data-changed", "launch_item");

    Ok(LaunchResult {
        ok: true,
        message: format!("launched: {name}"),
    })
}

#[tauri::command]
async fn open_editor(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    item_id: Option<String>,
) -> Result<(), String> {
    let item = {
        let data = state.data.lock().map_err(|_| "state lock poisoned".to_string())?;
        let group = data
            .groups
            .iter()
            .find(|g| g.id == group_id)
            .ok_or_else(|| "group not found".to_string())?;

        match item_id {
            Some(ref wanted_id) => group.items.iter().find(|it| it.id == *wanted_id).cloned(),
            None => None,
        }
    };

    {
        let mut ctx = state
            .editor_context
            .lock()
            .map_err(|_| "editor context lock poisoned".to_string())?;
        *ctx = Some(EditorContext { group_id, item });
    }

    if let Some(editor) = app.get_webview_window("editor") {
        editor.show().map_err(|e| e.to_string())?;
        editor.set_focus().map_err(|e| e.to_string())?;
        let _ = editor.emit("editor-context-changed", "updated");
        return Ok(());
    }

    WebviewWindowBuilder::new(&app, "editor", WebviewUrl::App("editor.html".into()))
        .title("编辑启动项")
        .inner_size(560.0, 460.0)
        .decorations(false)
        .resizable(true)
        .build()
        .map_err(|e| format!("create editor window failed: {e}"))?;

    Ok(())
}

#[tauri::command]
fn get_editor_context(state: tauri::State<'_, AppState>) -> Result<Option<EditorContext>, String> {
    let ctx = state
        .editor_context
        .lock()
        .map_err(|_| "editor context lock poisoned".to_string())?;
    Ok(ctx.clone())
}

#[tauri::command]
fn close_editor(app: AppHandle) -> Result<(), String> {
    if let Some(editor) = app.get_webview_window("editor") {
        editor.close().map_err(|e| e.to_string())?;
    }
    Ok(())
}

#[tauri::command]
fn update_settings(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    settings: Settings,
) -> Result<(), String> {
    {
        let mut current = state
            .settings
            .lock()
            .map_err(|_| "settings lock poisoned".to_string())?;
        *current = settings;
    }

    state.save_settings()?;
    let _ = app.emit("settings-changed", "updated");
    Ok(())
}

#[tauri::command]
fn add_group(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    name: String,
) -> Result<String, String> {
    let group_name = name.trim();
    if group_name.is_empty() {
        return Err("group name is empty".to_string());
    }

    let new_group_id = {
        let mut data = state
            .data
            .lock()
            .map_err(|_| "state lock poisoned".to_string())?;

        if data
            .groups
            .iter()
            .any(|group| group.name.eq_ignore_ascii_case(group_name))
        {
            return Err("group already exists".to_string());
        }

        let next_order = data
            .groups
            .iter()
            .map(|group| group.order)
            .max()
            .unwrap_or(-1)
            + 1;

        let group_id = generate_id("group");
        data.groups.push(Group {
            id: group_id.clone(),
            name: group_name.to_string(),
            order: next_order,
            items: Vec::new(),
        });
        group_id
    };

    state.save_data()?;
    let _ = app.emit("data-changed", "add_group");
    Ok(new_group_id)
}

#[tauri::command]
fn rename_group(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    name: String,
) -> Result<(), String> {
    let next_name = name.trim();
    if next_name.is_empty() {
        return Err("group name is empty".to_string());
    }

    {
        let mut data = state
            .data
            .lock()
            .map_err(|_| "state lock poisoned".to_string())?;

        if data
            .groups
            .iter()
            .any(|group| group.id != group_id && group.name.eq_ignore_ascii_case(next_name))
        {
            return Err("group already exists".to_string());
        }

        let group = data
            .groups
            .iter_mut()
            .find(|group| group.id == group_id)
            .ok_or_else(|| "group not found".to_string())?;

        group.name = next_name.to_string();
    }

    state.save_data()?;
    let _ = app.emit("data-changed", "rename_group");
    Ok(())
}

#[tauri::command]
fn move_item(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    item_id: String,
    target_group_id: String,
) -> Result<(), String> {
    if group_id == target_group_id {
        return Err("source and target group are the same".to_string());
    }

    {
        let mut data = state
            .data
            .lock()
            .map_err(|_| "state lock poisoned".to_string())?;

        let from_index = data
            .groups
            .iter()
            .position(|group| group.id == group_id)
            .ok_or_else(|| "source group not found".to_string())?;

        let to_index = data
            .groups
            .iter()
            .position(|group| group.id == target_group_id)
            .ok_or_else(|| "target group not found".to_string())?;

        let moved_item = {
            let from_group = data
                .groups
                .get_mut(from_index)
                .ok_or_else(|| "source group not found".to_string())?;
            let item_index = from_group
                .items
                .iter()
                .position(|item| item.id == item_id)
                .ok_or_else(|| "item not found".to_string())?;
            from_group.items.remove(item_index)
        };

        let to_group = data
            .groups
            .get_mut(to_index)
            .ok_or_else(|| "target group not found".to_string())?;
        to_group.items.push(moved_item);
    }

    state.save_data()?;
    let _ = app.emit("data-changed", "move_item");
    Ok(())
}

fn expand_windows_env_vars(input: &str) -> String {
    let mut result = String::new();
    let mut rest = input;

    while let Some(start) = rest.find('%') {
        result.push_str(&rest[..start]);
        let remain = &rest[start + 1..];
        if let Some(end) = remain.find('%') {
            let key = &remain[..end];
            if key.is_empty() {
                result.push('%');
            } else if let Ok(value) = std::env::var(key) {
                result.push_str(&value);
            } else {
                result.push('%');
                result.push_str(key);
                result.push('%');
            }
            rest = &remain[end + 1..];
        } else {
            result.push('%');
            result.push_str(remain);
            return result;
        }
    }

    result.push_str(rest);
    result
}

fn resolve_icon_file_path(raw: &str) -> String {
    let mut normalized = raw.trim().trim_matches('"').trim().to_string();
    if let Some((left, _)) = normalized.split_once(',') {
        let candidate = left.trim().trim_matches('"').trim();
        if !candidate.is_empty() {
            normalized = candidate.to_string();
        }
    }

    normalized = expand_windows_env_vars(&normalized);
    if normalized.is_empty() {
        return normalized;
    }

    if normalized.contains('\\') || normalized.contains('/') || normalized.contains(':') {
        return normalized;
    }

    #[cfg(target_os = "windows")]
    if let Some(found) = resolve_executable_with_search_path(&normalized) {
        return found;
    }

    normalized
}

#[cfg(target_os = "windows")]
fn resolve_executable_with_search_path(file_name: &str) -> Option<String> {
    use windows_sys::Win32::Storage::FileSystem::SearchPathW;

    let file_name_w = encode_wide_null(file_name);
    let required = unsafe {
        SearchPathW(
            ptr::null(),
            file_name_w.as_ptr(),
            ptr::null(),
            0,
            ptr::null_mut(),
            ptr::null_mut(),
        )
    };
    if required == 0 {
        return None;
    }

    let mut buffer = vec![0u16; required as usize + 1];
    let written = unsafe {
        SearchPathW(
            ptr::null(),
            file_name_w.as_ptr(),
            ptr::null(),
            buffer.len() as u32,
            buffer.as_mut_ptr(),
            ptr::null_mut(),
        )
    };
    if written == 0 {
        return None;
    }

    let value = String::from_utf16_lossy(&buffer[..written as usize])
        .trim()
        .to_string();
    if value.is_empty() {
        None
    } else {
        Some(value)
    }
}

fn stable_hash_hex(input: &str) -> String {
    let mut hash: u64 = 0xcbf29ce484222325;
    for byte in input.as_bytes() {
        hash ^= *byte as u64;
        hash = hash.wrapping_mul(0x100000001b3);
    }
    format!("{hash:016x}")
}

#[cfg(target_os = "windows")]
fn encode_wide_null(input: &str) -> Vec<u16> {
    use std::os::windows::ffi::OsStrExt;
    std::ffi::OsStr::new(input)
        .encode_wide()
        .chain(std::iter::once(0))
        .collect()
}

fn run_win32_extract_to_ico(source: &str, output_file: &Path) -> Result<(), String> {
    #[cfg(target_os = "windows")]
    {
        use windows_sys::Win32::Foundation::HWND;
        use windows_sys::Win32::Graphics::Gdi::{
            BI_RGB, BITMAPINFO, BITMAPINFOHEADER, CreateCompatibleDC, CreateDIBSection,
            DIB_RGB_COLORS, DeleteDC, DeleteObject, GetDC, HBITMAP, HDC, HGDIOBJ, RGBQUAD,
            ReleaseDC, SelectObject,
        };
        use windows_sys::Win32::UI::{
            Shell::ExtractIconExW,
            WindowsAndMessaging::{
                DI_NORMAL, DestroyIcon, DrawIconEx, GetSystemMetrics, HICON, SM_CXICON, SM_CYICON,
            },
        };

        let source_w = encode_wide_null(source);
        let mut large_icons: [HICON; 1] = [ptr::null_mut()];
        let extracted = unsafe {
            ExtractIconExW(
                source_w.as_ptr(),
                0,
                large_icons.as_mut_ptr(),
                ptr::null_mut(),
                1,
            )
        };
        if extracted == 0 || large_icons[0].is_null() {
            return Err("extract icon failed: icon not found".to_string());
        }

        let icon: HICON = large_icons[0];
        let width = unsafe { GetSystemMetrics(SM_CXICON) };
        let height = unsafe { GetSystemMetrics(SM_CYICON) };
        if width <= 0 || height <= 0 {
            unsafe {
                DestroyIcon(icon);
            }
            return Err("extract icon failed: invalid icon size".to_string());
        }

        let screen_dc: HDC = unsafe { GetDC(HWND::default()) };
        if screen_dc.is_null() {
            unsafe {
                DestroyIcon(icon);
            }
            return Err("extract icon failed: GetDC failed".to_string());
        }

        let mem_dc = unsafe { CreateCompatibleDC(screen_dc) };
        if mem_dc.is_null() {
            unsafe {
                ReleaseDC(HWND::default(), screen_dc);
                DestroyIcon(icon);
            }
            return Err("extract icon failed: CreateCompatibleDC failed".to_string());
        }

        let mut bmi = BITMAPINFO {
            bmiHeader: BITMAPINFOHEADER {
                biSize: std::mem::size_of::<BITMAPINFOHEADER>() as u32,
                biWidth: width,
                biHeight: -height,
                biPlanes: 1,
                biBitCount: 32,
                biCompression: BI_RGB,
                biSizeImage: 0,
                biXPelsPerMeter: 0,
                biYPelsPerMeter: 0,
                biClrUsed: 0,
                biClrImportant: 0,
            },
            bmiColors: [RGBQUAD {
                rgbBlue: 0,
                rgbGreen: 0,
                rgbRed: 0,
                rgbReserved: 0,
            }],
        };

        let mut bits_ptr = ptr::null_mut();
        let dib: HBITMAP = unsafe {
            CreateDIBSection(
                mem_dc,
                &mut bmi,
                DIB_RGB_COLORS,
                &mut bits_ptr,
                ptr::null_mut(),
                0,
            )
        };
        if dib.is_null() || bits_ptr.is_null() {
            unsafe {
                DeleteDC(mem_dc);
                ReleaseDC(HWND::default(), screen_dc);
                DestroyIcon(icon);
            }
            return Err("extract icon failed: CreateDIBSection failed".to_string());
        }

        let old_obj = unsafe { SelectObject(mem_dc, dib as HGDIOBJ) };
        let drawn = unsafe {
            DrawIconEx(
                mem_dc,
                0,
                0,
                icon,
                width,
                height,
                0,
                ptr::null_mut(),
                DI_NORMAL,
            )
        };

        let size = (width as usize) * (height as usize) * 4;
        let bgra = unsafe { std::slice::from_raw_parts(bits_ptr as *const u8, size) };
        let mut rgba = Vec::with_capacity(size);
        for chunk in bgra.chunks_exact(4) {
            rgba.push(chunk[2]);
            rgba.push(chunk[1]);
            rgba.push(chunk[0]);
            rgba.push(chunk[3]);
        }

        unsafe {
            SelectObject(mem_dc, old_obj);
            DeleteObject(dib as HGDIOBJ);
            DeleteDC(mem_dc);
            ReleaseDC(HWND::default(), screen_dc);
            DestroyIcon(icon);
        }

        if drawn == 0 {
            return Err("extract icon failed: DrawIconEx failed".to_string());
        }

        let image = ico::IconImage::from_rgba_data(width as u32, height as u32, rgba);
        let entry = ico::IconDirEntry::encode(&image).map_err(|e| format!("encode icon failed: {e}"))?;
        let mut dir = ico::IconDir::new(ico::ResourceType::Icon);
        dir.add_entry(entry);

        if let Some(parent) = output_file.parent() {
            fs::create_dir_all(parent).map_err(|e| format!("create icons dir failed: {e}"))?;
        }
        let mut bytes = Vec::new();
        dir.write(Cursor::new(&mut bytes))
            .map_err(|e| format!("write ico failed: {e}"))?;
        fs::write(output_file, bytes).map_err(|e| format!("save ico failed: {e}"))?;

        Ok(())
    }

    #[cfg(not(target_os = "windows"))]
    {
        let _ = (source, output_file);
        Err("extract exe icon is only supported on Windows".to_string())
    }
}

fn build_item_icons(state: &AppState, groups: &[Group]) -> BTreeMap<String, String> {
    let mut result = BTreeMap::<String, String>::new();
    let mut indexed_paths = BTreeSet::<String>::new();

    let _ = fs::create_dir_all(&state.icons_dir);

    for group in groups {
        for item in &group.items {
            if item.item_type == "separator" {
                continue;
            }

            let source_raw = if item.icon_location.trim().is_empty() {
                item.target_path.as_str()
            } else {
                item.icon_location.as_str()
            };

            let resolved_path = resolve_icon_file_path(source_raw);
            if resolved_path.trim().is_empty() {
                continue;
            }

            indexed_paths.insert(resolved_path.clone());

            let hash = stable_hash_hex(&resolved_path.to_ascii_lowercase());
            let ico_path = state.icons_dir.join(format!("{hash}.ico"));

            if !ico_path.exists() {
                if run_win32_extract_to_ico(&resolved_path, &ico_path).is_err() {
                    continue;
                }
            }

            let cache_key = hash.clone();
            let cached = state
                .icon_cache
                .lock()
                .ok()
                .and_then(|cache| cache.get(&cache_key).cloned());

            let data_url = if let Some(value) = cached {
                value
            } else {
                let bytes = match fs::read(&ico_path) {
                    Ok(value) => value,
                    Err(_) => continue,
                };
                use base64::Engine;
                let encoded = base64::engine::general_purpose::STANDARD.encode(bytes);
                let value = format!("data:image/x-icon;base64,{encoded}");
                if let Ok(mut cache) = state.icon_cache.lock() {
                    cache.insert(cache_key, value.clone());
                }
                value
            };

            result.insert(item.id.clone(), data_url);
        }
    }

    let mut lines = String::new();
    for path in indexed_paths {
        lines.push_str(&path);
        lines.push('\n');
    }
    let _ = write_text_if_changed(&state.icons_index_path, &lines);

    result
}

#[tauri::command]
fn extract_exe_icon(state: tauri::State<'_, AppState>, path: String) -> Result<String, String> {
    let normalized = resolve_icon_file_path(&path);
    if normalized.trim().is_empty() {
        return Err("icon path is empty".to_string());
    }

    let hash = stable_hash_hex(&normalized.to_ascii_lowercase());
    let ico_path = state.icons_dir.join(format!("{hash}.ico"));
    if !ico_path.exists() {
        run_win32_extract_to_ico(&normalized, &ico_path)?;
    }

    let bytes = fs::read(&ico_path).map_err(|e| format!("read icon failed: {e}"))?;
    use base64::Engine;
    let encoded = base64::engine::general_purpose::STANDARD.encode(bytes);
    Ok(format!("data:image/x-icon;base64,{encoded}"))
}

/*
legacy per-item extraction command remains available for compatibility,
but runtime UI now relies on build_item_icons via load_launcher_state.
*/

fn toggle_main_window(app: &AppHandle) {
    if let Some(main) = app.get_webview_window("main") {
        match main.is_visible() {
            Ok(true) => {
                let _ = main.hide();
            }
            Ok(false) | Err(_) => {
                let _ = main.show();
                let _ = main.set_focus();
            }
        }
    }
}

fn register_hotkey(app: &AppHandle, shortcut: &str) {
    let manager = app.global_shortcut();
    let _ = manager.unregister_all();
    let _ = manager.on_shortcut(shortcut, move |app_handle, _shortcut, event| {
        if event.state == ShortcutState::Pressed {
            toggle_main_window(app_handle);
        }
    });
}

fn build_tray(app: &AppHandle) -> tauri::Result<TrayIcon> {
    let toggle = MenuItem::with_id(app, "toggle", "显示 / 隐藏", true, None::<&str>)?;
    let quit = MenuItem::with_id(app, "quit", "退出", true, None::<&str>)?;
    let menu = Menu::with_items(app, &[&toggle, &quit])?;

    let mut tray_builder = TrayIconBuilder::new().menu(&menu).show_menu_on_left_click(false);
    if let Some(icon) = app.default_window_icon() {
        tray_builder = tray_builder.icon(icon.clone());
    }

    tray_builder
        .on_menu_event(|app_handle, event| match event.id().as_ref() {
            "toggle" => toggle_main_window(app_handle),
            "quit" => app_handle.exit(0),
            _ => {}
        })
        .on_tray_icon_event(|tray, event| {
            if let TrayIconEvent::Click {
                button: MouseButton::Left,
                button_state: MouseButtonState::Up,
                ..
            } = event
            {
                toggle_main_window(tray.app_handle());
            }
        })
        .build(app)
}

fn main() {
    let (data, settings, data_path, settings_path) = load_state();
    let mut run_dir = std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."));
    if run_dir
        .file_name()
        .and_then(|name| name.to_str())
        .is_some_and(|name| name.eq_ignore_ascii_case("src-tauri"))
    {
        if let Some(parent) = run_dir.parent() {
            run_dir = parent.to_path_buf();
        }
    }
    let icons_dir = run_dir.join("icons");
    let icons_index_path = icons_dir.join("index.txt");

    let state = AppState {
        data: Mutex::new(data),
        settings: Mutex::new(settings),
        editor_context: Mutex::new(None),
        tray_icon: Mutex::new(None),
        icon_cache: Mutex::new(HashMap::new()),
        icons_dir,
        icons_index_path,
        data_path,
        settings_path,
    };

    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_global_shortcut::Builder::new().build())
        .manage(state)
        .setup(|app: &mut tauri::App| {
            let state: tauri::State<'_, AppState> = app.state();
            let settings = state
                .settings
                .lock()
                .map_err(|_| tauri::Error::AssetNotFound("settings lock poisoned".to_string()))?
                .clone();

            let tray = build_tray(app.app_handle())?;
            if let Ok(mut slot) = state.tray_icon.lock() {
                *slot = Some(tray);
            }

            register_hotkey(&app.app_handle(), &settings.hotkey);
            Ok(())
        })
        .on_window_event(|window: &Window, event: &WindowEvent| {
            if window.label() == "main" {
                if let WindowEvent::CloseRequested { api, .. } = event {
                    api.prevent_close();
                    let _ = window.hide();
                }
            } else if window.label() == "editor" {
                if let WindowEvent::CloseRequested { .. } = event {
                    if let Ok(mut state) = window.state::<AppState>().editor_context.lock() {
                        *state = None;
                    }
                }
            }
        })
        .invoke_handler(tauri::generate_handler![
            load_launcher_state,
            upsert_item,
            delete_item,
            launch_item,
            move_item,
            extract_exe_icon,
            add_group,
            rename_group,
            open_editor,
            get_editor_context,
            close_editor,
            update_settings
        ])
        .run(tauri::generate_context!())
        .unwrap_or_else(|e| {
            eprintln!("error while running tauri application: {e}");
            panic!("error while running tauri application");
        });
}
