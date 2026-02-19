use tauri::WebviewWindow;

#[cfg(target_os = "windows")]
pub fn disable_window_round_corners(window: &WebviewWindow) {
    use raw_window_handle::{HasWindowHandle, RawWindowHandle};
    use windows_sys::Win32::Foundation::HWND;
    use windows_sys::Win32::Graphics::Dwm::{
        DwmSetWindowAttribute, DWMWA_WINDOW_CORNER_PREFERENCE, DWMWCP_DONOTROUND,
    };

    let Ok(handle) = window.window_handle() else {
        return;
    };
    let RawWindowHandle::Win32(raw) = handle.as_raw() else {
        return;
    };

    let preference: i32 = DWMWCP_DONOTROUND;
    let hwnd: HWND = raw.hwnd.get() as HWND;
    unsafe {
        let _ = DwmSetWindowAttribute(
            hwnd,
            DWMWA_WINDOW_CORNER_PREFERENCE as u32,
            &preference as *const _ as _,
            std::mem::size_of::<i32>() as u32,
        );
    }
}

#[cfg(target_os = "windows")]
pub fn enable_window_shadow(window: &WebviewWindow) {
    use raw_window_handle::{HasWindowHandle, RawWindowHandle};
    use windows_sys::Win32::Foundation::HWND;
    use windows_sys::Win32::UI::WindowsAndMessaging::{
        GetClassLongPtrW, SetClassLongPtrW, CS_DROPSHADOW, GCL_STYLE,
    };

    let Ok(handle) = window.window_handle() else {
        return;
    };
    let RawWindowHandle::Win32(raw) = handle.as_raw() else {
        return;
    };

    let hwnd: HWND = raw.hwnd.get() as HWND;
    unsafe {
        let style = GetClassLongPtrW(hwnd, GCL_STYLE) as usize;
        let next_style = style | CS_DROPSHADOW as usize;
        let _ = SetClassLongPtrW(hwnd, GCL_STYLE, next_style as isize);
    }
}

#[cfg(not(target_os = "windows"))]
pub fn enable_window_shadow(_window: &WebviewWindow) {}

#[cfg(not(target_os = "windows"))]
pub fn disable_window_round_corners(_window: &WebviewWindow) {}
