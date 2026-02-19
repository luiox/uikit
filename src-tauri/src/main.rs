#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

use serde::Serialize;
use std::{
    collections::BTreeMap,
    fs,
    io::Cursor,
    path::{Path, PathBuf},
    ptr,
    sync::atomic::{AtomicU64, Ordering},
    time::{SystemTime, UNIX_EPOCH},
};
use tauri::{
    menu::{Menu, MenuItem},
    tray::{MouseButton, MouseButtonState, TrayIcon, TrayIconBuilder, TrayIconEvent},
    AppHandle, Manager, Window, WindowEvent,
};
use tauri_plugin_global_shortcut::{GlobalShortcutExt, ShortcutState};

mod models;
mod commands;
mod state;
mod window_style;

use commands::{
    add_group, close_editor, delete_item, extract_exe_icon, get_editor_context, launch_item,
    load_launcher_state, move_item, open_editor, rename_group, update_settings, upsert_item,
};
use models::{
    default_group_panel_width, default_main_window_height, default_main_window_width, Group,
    LaunchItem, LauncherData, LegacyItem, Settings,
};
use state::AppState;
use window_style::{disable_window_round_corners, enable_window_shadow};

static ID_COUNTER: AtomicU64 = AtomicU64::new(1);

pub(crate) fn generate_id(prefix: &str) -> String {
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_millis())
        .unwrap_or(0);
    let idx = ID_COUNTER.fetch_add(1, Ordering::Relaxed);
    format!("{prefix}_{now}_{idx}")
}

pub(crate) fn is_separator_item(name: &str, target: &str, icon: &str) -> bool {
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
        group_panel_width: default_group_panel_width(),
        main_window_width: default_main_window_width(),
        main_window_height: default_main_window_height(),
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

pub(crate) fn write_json_atomic<T: Serialize>(path: &Path, value: &T) -> Result<(), String> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).map_err(|e| format!("create dir failed: {e}"))?;
    }
    let tmp = path.with_extension("json.tmp");
    let content = serde_json::to_string_pretty(value).map_err(|e| format!("serialize failed: {e}"))?;
    fs::write(&tmp, content).map_err(|e| format!("write tmp failed: {e}"))?;
    fs::rename(&tmp, path).map_err(|e| format!("replace file failed: {e}"))?;
    Ok(())
}

pub(crate) fn write_text_if_changed(path: &Path, content: &str) -> Result<(), String> {
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

pub(crate) fn split_windows_args(arguments: &str) -> Vec<String> {
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
                        group_panel_width: default_group_panel_width(),
                        main_window_width: default_main_window_width(),
                        main_window_height: default_main_window_height(),
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
                group_panel_width: default_group_panel_width(),
                main_window_width: default_main_window_width(),
                main_window_height: default_main_window_height(),
            }
        };
        let _ = write_json_atomic(&settings_path, &s);
        s
    };

    (launcher_data, settings, data_path, settings_path)
}

// command handlers moved to commands.rs

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

pub(crate) fn resolve_icon_file_path(raw: &str) -> String {
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

pub(crate) fn stable_hash_hex(input: &str) -> String {
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

pub(crate) fn run_win32_extract_to_ico(source: &str, output_file: &Path) -> Result<(), String> {
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

// icon commands moved to commands.rs

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

    let state = AppState::new(
        data,
        settings,
        icons_dir,
        icons_index_path,
        data_path,
        settings_path,
    );

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

            if let Some(main) = app.get_webview_window("main") {
                let width = settings.main_window_width.round().max(640.0) as u32;
                let height = settings.main_window_height.round().max(420.0) as u32;
                let _ = main.set_size(tauri::Size::Physical(tauri::PhysicalSize::new(width, height)));
                disable_window_round_corners(&main);
                enable_window_shadow(&main);
            }
            Ok(())
        })
        .on_window_event(|window: &Window, event: &WindowEvent| {
            if window.label() == "main" {
                match event {
                    WindowEvent::Resized(size) => {
                        let state = window.state::<AppState>();
                        {
                            if let Ok(mut settings) = state.settings.lock() {
                                settings.main_window_width = size.width as f64;
                                settings.main_window_height = size.height as f64;
                            }
                        }
                        let _ = state.save_settings();
                    }
                    WindowEvent::CloseRequested { api, .. } => {
                        api.prevent_close();
                        let _ = window.hide();
                    }
                    _ => {}
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
