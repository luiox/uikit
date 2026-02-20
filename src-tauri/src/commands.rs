use std::{
    collections::{BTreeMap, BTreeSet},
    fs,
    path::Path,
    process::Command,
};

use tauri::{AppHandle, Emitter, Manager, WebviewUrl, WebviewWindowBuilder};

use crate::{
    generate_id, is_separator_item, resolve_icon_file_path, run_win32_extract_to_ico,
    split_windows_args, stable_hash_hex, write_text_if_changed,
};
use crate::models::{EditorContext, Group, ItemInput, LaunchItem, LauncherState, LaunchResult, Settings};
use crate::state::AppState;
use crate::window_style::{disable_window_round_corners, enable_window_shadow};

#[tauri::command]
pub fn load_launcher_state(state: tauri::State<'_, AppState>) -> Result<LauncherState, String> {
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
pub fn upsert_item(
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
pub fn delete_item(
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
pub fn launch_item(
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
pub async fn open_editor(
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

    let editor = WebviewWindowBuilder::new(&app, "editor", WebviewUrl::App("editor.html".into()))
        .title("编辑启动项")
        .inner_size(560.0, 460.0)
        .decorations(false)
        .resizable(true)
        .build()
        .map_err(|e| format!("create editor window failed: {e}"))?;

    disable_window_round_corners(&editor);
    enable_window_shadow(&editor);

    Ok(())
}

#[tauri::command]
pub fn get_editor_context(state: tauri::State<'_, AppState>) -> Result<Option<EditorContext>, String> {
    let ctx = state
        .editor_context
        .lock()
        .map_err(|_| "editor context lock poisoned".to_string())?;
    Ok(ctx.clone())
}

#[tauri::command]
pub fn close_editor(app: AppHandle) -> Result<(), String> {
    if let Some(editor) = app.get_webview_window("editor") {
        editor.close().map_err(|e| e.to_string())?;
    }
    Ok(())
}

#[tauri::command]
pub fn update_settings(
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
pub fn add_group(
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
pub fn rename_group(
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
pub fn move_item(
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

#[tauri::command]
pub fn extract_exe_icon(state: tauri::State<'_, AppState>, path: String) -> Result<String, String> {
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

#[tauri::command]
pub fn create_items_from_dropped_paths(
    app: AppHandle,
    state: tauri::State<'_, AppState>,
    group_id: String,
    paths: Vec<String>,
) -> Result<usize, String> {
    if paths.is_empty() {
        return Ok(0);
    }

    let mut created = 0usize;

    {
        let mut data = state
            .data
            .lock()
            .map_err(|_| "state lock poisoned".to_string())?;

        let group = data
            .groups
            .iter_mut()
            .find(|g| g.id == group_id)
            .ok_or_else(|| "group not found".to_string())?;

        for raw_path in paths {
            let (target_path, icon_location, arguments) = resolve_dropped_target_and_icon(&raw_path);
            if target_path.trim().is_empty() {
                continue;
            }

            let name = basename_of_path(&target_path);
            if name.trim().is_empty() {
                continue;
            }

            group.items.push(LaunchItem {
                id: generate_id("item"),
                item_type: "app".to_string(),
                name,
                target_path,
                icon_location,
                arguments,
                launch_count: 0,
                enabled: true,
            });
            created = created.saturating_add(1);
        }
    }

    if created > 0 {
        state.save_data()?;
        let _ = app.emit("data-changed", "create_items_from_dropped_paths");
    }

    Ok(created)
}

fn basename_of_path(path: &str) -> String {
    Path::new(path)
    .file_stem()
        .and_then(|value| value.to_str())
        .map(|value| value.to_string())
        .unwrap_or_else(|| path.to_string())
}

fn resolve_dropped_target_and_icon(raw_path: &str) -> (String, String, String) {
    let dropped = normalize_dropped_path(raw_path);
    if dropped.is_empty() {
        return (String::new(), String::new(), String::new());
    }

    #[cfg(target_os = "windows")]
    {
        if dropped.to_ascii_lowercase().ends_with(".lnk") {
            if let Some((target, args)) = resolve_windows_shortcut_target(&dropped) {
                let normalized = target.trim().trim_matches('"').trim().to_string();
                if !normalized.is_empty() {
                    return (normalized.clone(), normalized, args);
                }
            }
            return (String::new(), String::new(), String::new());
        }
    }

    (dropped.clone(), dropped, String::new())
}

fn normalize_dropped_path(raw_path: &str) -> String {
    let mut value = raw_path.trim().trim_matches('"').trim().to_string();
    if value.is_empty() {
        return value;
    }

    let lower = value.to_ascii_lowercase();
    if lower.starts_with("file://") {
        let without_scheme = value
            .trim_start_matches("file:///")
            .trim_start_matches("file://")
            .to_string();
        let slash_fixed = without_scheme.replace('/', "\\");
        value = percent_decode_path(&slash_fixed);
    }

    value
}

fn percent_decode_path(input: &str) -> String {
    let bytes = input.as_bytes();
    let mut out = Vec::with_capacity(bytes.len());
    let mut index = 0usize;

    while index < bytes.len() {
        if bytes[index] == b'%' && index + 2 < bytes.len() {
            let hi = bytes[index + 1] as char;
            let lo = bytes[index + 2] as char;
            let hex = [hi, lo].iter().collect::<String>();
            if let Ok(decoded) = u8::from_str_radix(&hex, 16) {
                out.push(decoded);
                index += 3;
                continue;
            }
        }
        out.push(bytes[index]);
        index += 1;
    }

    String::from_utf8_lossy(&out).to_string()
}

#[cfg(target_os = "windows")]
fn resolve_windows_shortcut_target(shortcut_path: &str) -> Option<(String, String)> {
    let escaped_path = shortcut_path.replace('\'', "''");
    let script = format!(
        "$ErrorActionPreference = 'Stop'; \
         $ws = New-Object -ComObject WScript.Shell; \
         $shortcut = $ws.CreateShortcut('{escaped_path}'); \
         $targetRaw = [string]$shortcut.TargetPath; \
         $argsRaw = [string]$shortcut.Arguments; \
         $targetB64 = [Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes($targetRaw)); \
         $argsB64 = [Convert]::ToBase64String([System.Text.Encoding]::UTF8.GetBytes($argsRaw)); \
         [Console]::OutputEncoding = [System.Text.Encoding]::UTF8; \
         Write-Output ($targetB64 + '|' + $argsB64)"
    );

    let mut raw = String::new();
    for shell in ["powershell.exe", "pwsh.exe"] {
        let output = match Command::new(shell)
            .args(["-NoProfile", "-NonInteractive", "-Command", &script])
            .output()
        {
            Ok(value) => value,
            Err(_) => continue,
        };
        if output.status.success() {
            raw = String::from_utf8_lossy(&output.stdout).trim().to_string();
            if !raw.is_empty() {
                break;
            }
        }
    }

    if raw.is_empty() {
        return None;
    }

    let (target_b64, args_b64) = raw.split_once('|')?;
    use base64::Engine;
    let target_bytes = base64::engine::general_purpose::STANDARD.decode(target_b64).ok()?;
    let args_bytes = base64::engine::general_purpose::STANDARD.decode(args_b64).ok()?;
    let target = String::from_utf8_lossy(&target_bytes).trim().to_string();
    let args = String::from_utf8_lossy(&args_bytes).trim().to_string();
    if target.is_empty() {
        None
    } else {
        Some((target, args))
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
