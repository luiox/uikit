#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

use serde::{Deserialize, Serialize};
use std::{
    collections::BTreeMap,
    fs,
    path::{Path, PathBuf},
    process::Command,
    sync::{atomic::{AtomicU64, Ordering}, Mutex},
    time::{SystemTime, UNIX_EPOCH},
};
use tauri::{
    AppHandle, CustomMenuItem, GlobalShortcutManager, Manager, SystemTray, SystemTrayEvent,
    SystemTrayMenu, WindowBuilder, WindowEvent, WindowUrl,
};

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

    Ok(LauncherState {
        groups: data.groups.clone(),
        settings: settings.clone(),
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

        if let Some(id) = item.id {
            if let Some(existing) = group.items.iter_mut().find(|it| it.id == id) {
                existing.item_type = item_type;
                existing.name = item.name;
                existing.target_path = item.target_path;
                existing.icon_location = item.icon_location;
                existing.arguments = item.arguments;
                existing.enabled = item.enabled.unwrap_or(true);
            } else {
                group.items.push(LaunchItem {
                    id,
                    item_type,
                    name: item.name,
                    target_path: item.target_path,
                    icon_location: item.icon_location,
                    arguments: item.arguments,
                    launch_count: 0,
                    enabled: item.enabled.unwrap_or(true),
                });
            }
        } else {
            group.items.push(LaunchItem {
                id: generate_id("item"),
                item_type,
                name: item.name,
                target_path: item.target_path,
                icon_location: item.icon_location,
                arguments: item.arguments,
                launch_count: 0,
                enabled: item.enabled.unwrap_or(true),
            });
        }
    }

    state.save_data()?;
    let _ = app.emit_all("data-changed", "upsert_item");
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
    let _ = app.emit_all("data-changed", "delete_item");
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
        if let Some(main) = app.get_window("main") {
            let _ = main.hide();
        }
    }

    let _ = app.emit_all("data-changed", "launch_item");

    Ok(LaunchResult {
        ok: true,
        message: format!("launched: {name}"),
    })
}

#[tauri::command]
fn open_editor(
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

    if let Some(editor) = app.get_window("editor") {
        editor.show().map_err(|e| e.to_string())?;
        editor.set_focus().map_err(|e| e.to_string())?;
        let _ = editor.emit("editor-context-changed", "updated");
        return Ok(());
    }

    WindowBuilder::new(&app, "editor", WindowUrl::App("editor.html".into()))
        .title("编辑启动项")
        .inner_size(560.0, 460.0)
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
    if let Some(editor) = app.get_window("editor") {
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
    let _ = app.emit_all("settings-changed", "updated");
    Ok(())
}

fn toggle_main_window(app: &AppHandle) {
    if let Some(main) = app.get_window("main") {
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
    let mut manager = app.global_shortcut_manager();
    let _ = manager.unregister_all();
    let app_handle = app.clone();
    let _ = manager.register(shortcut, move || {
        toggle_main_window(&app_handle);
    });
}

fn main() {
    let (data, settings, data_path, settings_path) = load_state();

    let state = AppState {
        data: Mutex::new(data),
        settings: Mutex::new(settings),
        editor_context: Mutex::new(None),
        data_path,
        settings_path,
    };

    let show_hide = CustomMenuItem::new("toggle".to_string(), "显示 / 隐藏");
    let quit = CustomMenuItem::new("quit".to_string(), "退出");
    let tray_menu = SystemTrayMenu::new().add_item(show_hide).add_item(quit);

    tauri::Builder::default()
        .manage(state)
        .system_tray(SystemTray::new().with_menu(tray_menu))
        .on_system_tray_event(|app, event| match event {
            SystemTrayEvent::MenuItemClick { id, .. } => match id.as_str() {
                "toggle" => toggle_main_window(app),
                "quit" => std::process::exit(0),
                _ => {}
            },
            SystemTrayEvent::LeftClick { .. } => toggle_main_window(app),
            _ => {}
        })
        .setup(|app: &mut tauri::App| {
            let state: tauri::State<'_, AppState> = app.state();
            let settings = state
                .settings
                .lock()
                .map_err(|_| tauri::Error::AssetNotFound("settings lock poisoned".to_string()))?
                .clone();
            register_hotkey(&app.app_handle(), &settings.hotkey);
            Ok(())
        })
        .on_window_event(|event: tauri::GlobalWindowEvent| {
            if event.window().label() == "main" {
                if let WindowEvent::CloseRequested { api, .. } = event.event() {
                    api.prevent_close();
                    let _ = event.window().hide();
                }
            }
        })
        .invoke_handler(tauri::generate_handler![
            load_launcher_state,
            upsert_item,
            delete_item,
            launch_item,
            open_editor,
            get_editor_context,
            close_editor,
            update_settings
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
