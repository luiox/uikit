use serde::{Deserialize, Serialize};
use std::{
    cell::RefCell,
    fs,
    path::PathBuf,
    process::Command,
    ptr::{self, null_mut},
};

#[cfg(target_os = "windows")]
use windows_sys::Win32::{
    Foundation::{HINSTANCE, HWND, LPARAM, LRESULT, RECT, WPARAM},
    Graphics::Gdi::{GetStockObject, DEFAULT_GUI_FONT, HFONT},
    System::LibraryLoader::GetModuleHandleW,
    UI::WindowsAndMessaging::{
        CreateWindowExW, DefWindowProcW, DispatchMessageW, GetClientRect, GetMessageW,
        LoadCursorW, MoveWindow, PostQuitMessage, RegisterClassW, SendMessageW, SetWindowTextW,
        ShowWindow, TranslateMessage,
        CS_HREDRAW, CS_VREDRAW, CW_USEDEFAULT, ES_LEFT, IDC_ARROW, LBN_DBLCLK, LBN_SELCHANGE,
        LB_ADDSTRING, LB_GETCURSEL, LB_RESETCONTENT, LB_SETCURSEL, LBS_NOTIFY, MSG, SW_SHOW,
        WM_COMMAND, WM_CREATE, WM_DESTROY, WM_SETFONT, WM_SIZE, WNDCLASSW, WS_BORDER, WS_CHILD,
        WS_EX_CLIENTEDGE, WS_OVERLAPPEDWINDOW, WS_TABSTOP, WS_VISIBLE,
    },
};

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
    current_group: Option<String>,
}

#[derive(Default)]
struct UiHandles {
    group_list: HWND,
    item_list: HWND,
    launch_btn: HWND,
    status_text: HWND,
}

struct AppState {
    data: LauncherData,
    settings: Settings,
    ui: UiHandles,
}

thread_local! {
    static APP_STATE: RefCell<Option<AppState>> = const { RefCell::new(None) };
}

const ID_GROUP_LIST: i32 = 1001;
const ID_ITEM_LIST: i32 = 1002;
const ID_LAUNCH_BTN: i32 = 1003;
const ID_STATUS: i32 = 1004;

fn wide_null(value: &str) -> Vec<u16> {
    value.encode_utf16().chain(std::iter::once(0)).collect()
}

fn app_base_dir() -> PathBuf {
    if let Some(dir) = dirs::data_local_dir() {
        let base = dir.join("nassistant");
        let _ = fs::create_dir_all(&base);
        return base;
    }
    std::env::current_dir().unwrap_or_else(|_| PathBuf::from("."))
}

fn default_launcher_data() -> LauncherData {
    LauncherData {
        version: 2,
        groups: vec![Group {
            id: "group_default".to_string(),
            name: "Common".to_string(),
            order: 0,
            items: Vec::new(),
        }],
    }
}

fn load_data() -> LauncherData {
    let path = app_base_dir().join("launcher.v2.json");
    if !path.exists() {
        return default_launcher_data();
    }
    fs::read_to_string(path)
        .ok()
        .and_then(|raw| serde_json::from_str::<LauncherData>(&raw).ok())
        .unwrap_or_else(default_launcher_data)
}

fn load_settings() -> Settings {
    let path = app_base_dir().join("nassistant.settings.json");
    if !path.exists() {
        return Settings { current_group: None };
    }
    fs::read_to_string(path)
        .ok()
        .and_then(|raw| serde_json::from_str::<Settings>(&raw).ok())
        .unwrap_or(Settings { current_group: None })
}

fn split_windows_args(arguments: &str) -> Vec<String> {
    let mut result = Vec::new();
    let mut current = String::new();
    let mut in_quotes = false;

    for ch in arguments.chars() {
        match ch {
            '"' => in_quotes = !in_quotes,
            ' ' | '\t' if !in_quotes => {
                if !current.is_empty() {
                    result.push(std::mem::take(&mut current));
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

fn with_state<T>(f: impl FnOnce(&mut AppState) -> T) -> Option<T> {
    APP_STATE.with(|state| {
        let mut borrow = state.borrow_mut();
        let value = borrow.as_mut()?;
        Some(f(value))
    })
}

#[cfg(target_os = "windows")]
unsafe fn set_status_text(text: &str) {
    let _ = with_state(|state| {
        if !state.ui.status_text.is_null() {
            let value = wide_null(text);
            SetWindowTextW(state.ui.status_text, value.as_ptr());
        }
    });
}

#[cfg(target_os = "windows")]
unsafe fn fill_items(group_index: i32) {
    let status = with_state(|state| {
        let list = state.ui.item_list;
        if list.is_null() {
            return None::<String>;
        }
        SendMessageW(list, LB_RESETCONTENT, 0, 0);

        let Ok(group_idx) = usize::try_from(group_index) else {
            return None;
        };
        if let Some(group) = state.data.groups.get(group_idx) {
            for item in &group.items {
                let name = wide_null(&item.name);
                SendMessageW(list, LB_ADDSTRING, 0, name.as_ptr() as LPARAM);
            }
            SendMessageW(list, LB_SETCURSEL, usize::MAX, 0);
            return Some(format!("分组: {}", group.name));
        }
        None
    })
    .flatten();

    if let Some(text) = status {
        set_status_text(&text);
    }
}

#[cfg(target_os = "windows")]
unsafe fn fill_groups() {
    let _ = with_state(|state| {
        let list = state.ui.group_list;
        if list.is_null() {
            return;
        }

        SendMessageW(list, LB_RESETCONTENT, 0, 0);
        for group in &state.data.groups {
            let name = wide_null(&group.name);
            SendMessageW(list, LB_ADDSTRING, 0, name.as_ptr() as LPARAM);
        }

        let mut selected = 0usize;
        if let Some(ref current_name) = state.settings.current_group {
            if let Some(idx) = state
                .data
                .groups
                .iter()
                .position(|group| group.name == *current_name)
            {
                selected = idx;
            }
        }

        SendMessageW(list, LB_SETCURSEL, selected, 0);
    });

    fill_items(with_state(|state| {
        let index = SendMessageW(state.ui.group_list, LB_GETCURSEL, 0, 0);
        i32::try_from(index).unwrap_or(0)
    })
    .unwrap_or(0));
}

#[cfg(target_os = "windows")]
unsafe fn launch_selected_item() {
    let maybe_item = with_state(|state| {
        let group_idx = SendMessageW(state.ui.group_list, LB_GETCURSEL, 0, 0);
        let item_idx = SendMessageW(state.ui.item_list, LB_GETCURSEL, 0, 0);

        let Ok(group_index) = usize::try_from(group_idx) else {
            return None;
        };
        let Ok(item_index) = usize::try_from(item_idx) else {
            return None;
        };

        state
            .data
            .groups
            .get(group_index)
            .and_then(|group| group.items.get(item_index))
            .cloned()
    })
    .flatten();

    let Some(item) = maybe_item else {
        set_status_text("请先选择一个启动项");
        return;
    };

    if item.item_type == "separator" {
        set_status_text("分隔项不能启动");
        return;
    }

    if item.target_path.trim().is_empty() {
        set_status_text("启动路径为空");
        return;
    }

    let mut command = Command::new(&item.target_path);
    let args = split_windows_args(&item.arguments);
    if !args.is_empty() {
        command.args(args);
    }

    match command.spawn() {
        Ok(_) => set_status_text(&format!("已启动: {}", item.name)),
        Err(error) => set_status_text(&format!("启动失败: {error}")),
    }
}

#[cfg(target_os = "windows")]
unsafe fn relayout(hwnd: HWND) {
    let mut rect: RECT = std::mem::zeroed();
    GetClientRect(hwnd, &mut rect);
    let width = rect.right - rect.left;
    let height = rect.bottom - rect.top;

    let left_width = 250;
    let top = 8;
    let gap = 8;
    let status_h = 24;
    let button_h = 28;
    let list_h = height - top - status_h - button_h - gap * 3;

    let _ = with_state(|state| {
        MoveWindow(state.ui.group_list, 8, top, left_width, list_h, 1);
        MoveWindow(
            state.ui.item_list,
            left_width + 16,
            top,
            width - left_width - 24,
            list_h,
            1,
        );
        MoveWindow(state.ui.launch_btn, width - 92, top + list_h + gap, 84, button_h, 1);
        MoveWindow(state.ui.status_text, 8, top + list_h + gap + 2, width - 108, status_h, 1);
    });
}

#[cfg(target_os = "windows")]
unsafe extern "system" fn wnd_proc(hwnd: HWND, msg: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
    match msg {
        WM_CREATE => {
            let font = GetStockObject(DEFAULT_GUI_FONT) as HFONT;

            let list_box_class = wide_null("LISTBOX");
            let button_class = wide_null("BUTTON");
            let static_class = wide_null("STATIC");
            let launch_text = wide_null("启动");
            let ready_text = wide_null("就绪");

            let group_list = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                list_box_class.as_ptr(),
                ptr::null(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | (LBS_NOTIFY as u32) | WS_BORDER,
                0,
                0,
                0,
                0,
                hwnd,
                ID_GROUP_LIST as usize as *mut core::ffi::c_void,
                null_mut(),
                null_mut(),
            );

            let item_list = CreateWindowExW(
                WS_EX_CLIENTEDGE,
                list_box_class.as_ptr(),
                ptr::null(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | (LBS_NOTIFY as u32) | WS_BORDER,
                0,
                0,
                0,
                0,
                hwnd,
                ID_ITEM_LIST as usize as *mut core::ffi::c_void,
                null_mut(),
                null_mut(),
            );

            let launch_btn = CreateWindowExW(
                0,
                button_class.as_ptr(),
                launch_text.as_ptr(),
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                0,
                0,
                0,
                0,
                hwnd,
                ID_LAUNCH_BTN as usize as *mut core::ffi::c_void,
                null_mut(),
                null_mut(),
            );

            let status_text = CreateWindowExW(
                0,
                static_class.as_ptr(),
                ready_text.as_ptr(),
                WS_CHILD | WS_VISIBLE | (ES_LEFT as u32),
                0,
                0,
                0,
                0,
                hwnd,
                ID_STATUS as usize as *mut core::ffi::c_void,
                null_mut(),
                null_mut(),
            );

            SendMessageW(group_list, WM_SETFONT, font as usize, 1);
            SendMessageW(item_list, WM_SETFONT, font as usize, 1);
            SendMessageW(launch_btn, WM_SETFONT, font as usize, 1);
            SendMessageW(status_text, WM_SETFONT, font as usize, 1);

            let _ = with_state(|state| {
                state.ui.group_list = group_list;
                state.ui.item_list = item_list;
                state.ui.launch_btn = launch_btn;
                state.ui.status_text = status_text;
            });

            relayout(hwnd);
            fill_groups();
            0
        }
        WM_SIZE => {
            relayout(hwnd);
            0
        }
        WM_COMMAND => {
            let id = (wparam & 0xFFFF) as i32;
            let notify = ((wparam >> 16) & 0xFFFF) as u32;

            if id == ID_GROUP_LIST && notify == LBN_SELCHANGE {
                let group_index = with_state(|state| SendMessageW(state.ui.group_list, LB_GETCURSEL, 0, 0))
                    .map(|v| i32::try_from(v).unwrap_or(-1))
                    .unwrap_or(-1);
                if group_index >= 0 {
                    fill_items(group_index);
                }
                return 0;
            }

            if id == ID_ITEM_LIST && notify == LBN_DBLCLK {
                launch_selected_item();
                return 0;
            }

            if id == ID_LAUNCH_BTN {
                launch_selected_item();
                return 0;
            }

            DefWindowProcW(hwnd, msg, wparam, lparam)
        }
        WM_DESTROY => {
            PostQuitMessage(0);
            0
        }
        _ => DefWindowProcW(hwnd, msg, wparam, lparam),
    }
}

#[cfg(target_os = "windows")]
fn main() {
    APP_STATE.with(|state| {
        *state.borrow_mut() = Some(AppState {
            data: load_data(),
            settings: load_settings(),
            ui: UiHandles::default(),
        });
    });

    unsafe {
        let instance: HINSTANCE = GetModuleHandleW(ptr::null());
        let class_name = wide_null("NAssistantWin32Launcher");
        let title = wide_null("nassistant (Win32)");

        let wc = WNDCLASSW {
            style: CS_HREDRAW | CS_VREDRAW,
            lpfnWndProc: Some(wnd_proc),
            hInstance: instance,
            lpszClassName: class_name.as_ptr(),
            hCursor: LoadCursorW(null_mut(), IDC_ARROW),
            ..std::mem::zeroed()
        };

        RegisterClassW(&wc);

        let hwnd = CreateWindowExW(
            0,
            class_name.as_ptr(),
            title.as_ptr(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            980,
            680,
            null_mut(),
            null_mut(),
            instance,
            null_mut(),
        );

        ShowWindow(hwnd, SW_SHOW);

        let mut msg: MSG = std::mem::zeroed();
        while GetMessageW(&mut msg, null_mut(), 0, 0) > 0 {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
}

#[cfg(not(target_os = "windows"))]
fn main() {
    eprintln!("nassistant-win32 is only available on Windows");
}
