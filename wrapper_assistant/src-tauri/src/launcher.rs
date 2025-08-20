use serde::Deserialize;

#[derive(Deserialize)]
pub struct LaunchProgramRequest {
    pub target_path: String,
    pub arguments: Option<String>,
    pub run_as_admin: bool,
}

pub fn launch_program_impl(req: LaunchProgramRequest) -> Result<(), String> {
    if req.run_as_admin {
        // Windows下用ShellExecuteW实现管理员提权启动
        #[cfg(target_os = "windows")]
        {
            use windows::Win32::UI::Shell::ShellExecuteW;
            use windows::Win32::Foundation::{HWND, HINSTANCE};
            use windows::core::PCWSTR;
            use std::ffi::OsStr;
            use std::os::windows::ffi::OsStrExt;
            use windows::Win32::UI::WindowsAndMessaging::SW_SHOWNORMAL;

            let exe: Vec<u16> = OsStr::new(&req.target_path).encode_wide().chain(Some(0)).collect();
            let args: Vec<u16> = req.arguments.as_deref().map(|s| OsStr::new(s).encode_wide().chain(Some(0)).collect()).unwrap_or_else(|| vec![0]);
            let verb: Vec<u16> = OsStr::new("runas").encode_wide().chain(Some(0)).collect();

            let result = unsafe {
                ShellExecuteW(
                    Some(HWND(std::ptr::null_mut())),
                    PCWSTR(verb.as_ptr()),
                    PCWSTR(exe.as_ptr()),
                    if args.len() > 1 { PCWSTR(args.as_ptr()) } else { PCWSTR(std::ptr::null()) },
                    PCWSTR(std::ptr::null()),
                    SW_SHOWNORMAL, 
                )
            };
            if (result.0 as usize) <= 32 {
                return Err(format!("ShellExecuteW failed: code {}", result.0 as usize));
            }
            Ok(())
        }
        #[cfg(not(target_os = "windows"))]
        {
            Err("管理员模式仅支持 Windows".to_string())
        }
    } else {
        // 普通启动
        let mut cmd = std::process::Command::new(&req.target_path);
        if let Some(args) = &req.arguments {
            // 简单分割参数，复杂场景可用 shell-words crate
            for arg in args.split_whitespace() {
                cmd.arg(arg);
            }
        }
        cmd.spawn()
            .map(|_| ())
            .map_err(|e| format!("启动失败: {}", e))
    }
}

// 获取当前程序的所在目录，以绝对路径
pub fn get_launcher_absolute_dir() -> String {
    std::env::current_exe()
        .ok()
        .and_then(|path| path.parent().map(|p| p.to_path_buf()))
        .map(|p| p.to_string_lossy().into_owned())
        .unwrap_or_else(|| "unknown".to_string())
}

// 获取当前程序所在盘符的根目录
pub fn get_launcher_root_dir() -> String {
    std::env::current_dir()
        .ok()
        .and_then(|path| path.components().next().map(|comp| comp.as_os_str().to_os_string()))
        .map(|os_str| os_str.to_string_lossy().into_owned())
        .unwrap_or_else(|| "unknown".to_string())
}

// %%rp%%替换为所在目录的绝对路径
// %%rr%%替换为所在盘的目录
pub fn fix_path(path: &str) -> String {
    let abs_dir = crate::launcher::get_launcher_absolute_dir();
    let root_dir = crate::launcher::get_launcher_root_dir();
    path.replace("%%rp%%", &abs_dir)
        .replace("%%rr%%", &root_dir)
}


// 图标提取相关
// 通过winapi提取目标路径exe的图标，并且转换为base64字符串
#[cfg(target_os = "windows")]
pub fn extract_icon_base64(exe_path: &str) -> Option<String> {
    use windows::Win32::UI::WindowsAndMessaging::{DestroyIcon, GetIconInfo, HICON};
    use windows::core::PCWSTR;
    #[link(name = "shell32")]
    extern "system" {
        fn ExtractIconW(hInst: windows::Win32::Foundation::HINSTANCE, lpszExeFileName: PCWSTR, nIconIndex: u32) -> HICON;
    }
    use windows::Win32::Graphics::Gdi::{GetObjectW, GetDIBits, BITMAP, BITMAPINFO, BITMAPINFOHEADER, DIB_RGB_COLORS, HBITMAP, CreateCompatibleDC, SelectObject, DeleteDC, DeleteObject, HGDIOBJ};
    use windows::Win32::Foundation::{HANDLE};
    use std::ptr::null_mut;
    use std::ffi::OsStr;
    use std::os::windows::ffi::OsStrExt;
    use image::{ImageBuffer, Rgba};
    use base64::engine::general_purpose::STANDARD as BASE64;
    use base64::Engine;

    // 转换路径为宽字符串
    let wide: Vec<u16> = OsStr::new(exe_path).encode_wide().chain(Some(0)).collect();
    // 获取主图标
    let hicon = unsafe { ExtractIconW(windows::Win32::Foundation::HINSTANCE(std::ptr::null_mut()), PCWSTR(wide.as_ptr()), 0) };
    if hicon.0.is_null() {
        return None;
    }

    // 获取图标信息
    let mut icon_info = unsafe { std::mem::zeroed() };
    if unsafe { GetIconInfo(hicon, &mut icon_info) }.is_err() {
        unsafe { let _ = DestroyIcon(hicon); };
        return None;
    }

    // 获取位图信息
    let mut bmp: BITMAP = unsafe { std::mem::zeroed() };
    if unsafe { GetObjectW(HGDIOBJ::from(icon_info.hbmColor), std::mem::size_of::<BITMAP>() as i32, Some(&mut bmp as *mut _ as *mut _)) } == 0 {
        unsafe {
            let _ = DestroyIcon(hicon);
            let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmColor));
            let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmMask));
        }
        return None;
    }

    let width = bmp.bmWidth as u32;
    let height = bmp.bmHeight as u32;
    let mut bi = BITMAPINFO {
        bmiHeader: BITMAPINFOHEADER {
            biSize: std::mem::size_of::<BITMAPINFOHEADER>() as u32,
            biWidth: width as i32,
            biHeight: -(height as i32), // top-down
            biPlanes: 1,
            biBitCount: 32,
            biCompression: 0,
            biSizeImage: 0,
            biXPelsPerMeter: 0,
            biYPelsPerMeter: 0,
            biClrUsed: 0,
            biClrImportant: 0,
        },
        bmiColors: [windows::Win32::Graphics::Gdi::RGBQUAD { rgbBlue: 0, rgbGreen: 0, rgbRed: 0, rgbReserved: 0 }; 1],
    };
    let mut pixels = vec![0u8; (width * height * 4) as usize];
    let hdc = unsafe { CreateCompatibleDC(None) };
    unsafe {
        SelectObject(hdc, HGDIOBJ::from(icon_info.hbmColor));
        GetDIBits(hdc, icon_info.hbmColor, 0, height as u32, Some(pixels.as_mut_ptr() as *mut _), &mut bi, DIB_RGB_COLORS);
        let _ = DeleteDC(hdc);
    }
    
    // 因为通道顺序不一样，需要调整
    // BGRA -> RGBA
    for chunk in pixels.chunks_exact_mut(4) {
        let b = chunk[0];
        let r = chunk[2];
        chunk[0] = r;
        chunk[2] = b;
    }

    // 转为 RGBA
    let img: ImageBuffer<Rgba<u8>, _> = match ImageBuffer::from_raw(width, height, pixels) {
        Some(img) => img,
        None => {
            unsafe {
                let _ = DestroyIcon(hicon);
                let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmColor));
                let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmMask));
            }
            return None;
        }
    };
    let dyn_img = image::DynamicImage::ImageRgba8(img);
    let mut png_bytes = std::io::Cursor::new(Vec::new());
    if dyn_img.write_to(&mut png_bytes, image::ImageFormat::Png).is_err() {
        unsafe {
            let _ = DestroyIcon(hicon);
            let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmColor));
            let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmMask));
        }
        return None;
    }

    unsafe {
        let _ = DestroyIcon(hicon);
        let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmColor));
        let _ = DeleteObject(HGDIOBJ::from(icon_info.hbmMask));
    }

    let b64 = BASE64.encode(&png_bytes.into_inner());
    Some(format!("data:image/png;base64,{}", b64))
}

#[cfg(not(target_os = "windows"))]
pub fn extract_icon_base64(_exe_path: &str) -> Option<String> {
    None
}
