#[cfg(target_os = "windows")]
fn main() {
    use std::ptr;
    use windows_sys::Win32::Foundation::{HINSTANCE, HWND, LPARAM, LRESULT, WPARAM};
    use windows_sys::Win32::System::LibraryLoader::GetModuleHandleW;
    use windows_sys::Win32::UI::WindowsAndMessaging::{
        CreateWindowExW, DefWindowProcW, DispatchMessageW, GetMessageW, LoadCursorW,
        PostQuitMessage, RegisterClassW, TranslateMessage, CS_HREDRAW, CS_VREDRAW, CW_USEDEFAULT,
        IDC_ARROW, MSG, WNDCLASSW, WM_DESTROY, WS_OVERLAPPEDWINDOW, WS_VISIBLE,
    };

    unsafe extern "system" fn wnd_proc(hwnd: HWND, msg: u32, wparam: WPARAM, lparam: LPARAM) -> LRESULT {
        match msg {
            WM_DESTROY => {
                PostQuitMessage(0);
                0
            }
            _ => DefWindowProcW(hwnd, msg, wparam, lparam),
        }
    }

    fn wide_null(value: &str) -> Vec<u16> {
        value.encode_utf16().chain(std::iter::once(0)).collect()
    }

    unsafe {
        let instance: HINSTANCE = GetModuleHandleW(ptr::null());
        let class_name = wide_null("ProbeWin32IdleClass");
        let title = wide_null("Probe Win32 Idle");

        let wnd = WNDCLASSW {
            style: CS_HREDRAW | CS_VREDRAW,
            lpfnWndProc: Some(wnd_proc),
            hInstance: instance,
            lpszClassName: class_name.as_ptr(),
            hCursor: LoadCursorW(ptr::null_mut(), IDC_ARROW),
            ..std::mem::zeroed()
        };

        RegisterClassW(&wnd);

        let _hwnd = CreateWindowExW(
            0,
            class_name.as_ptr(),
            title.as_ptr(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            640,
            420,
            ptr::null_mut(),
            ptr::null_mut(),
            instance,
            ptr::null_mut(),
        );

        let mut msg: MSG = std::mem::zeroed();
        while GetMessageW(&mut msg, ptr::null_mut(), 0, 0) > 0 {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
}

#[cfg(not(target_os = "windows"))]
fn main() {
    println!("probe-win32-idle is only available on Windows");
}
