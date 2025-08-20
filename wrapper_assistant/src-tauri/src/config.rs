use serde::{Deserialize, Serialize};
use std::fs;
use std::collections::HashMap;


#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct LaunchItem {
    pub name: String,
    pub target_path: String,
    pub icon_location: String,
    pub arguments: String,
    pub count: u32,
    pub icon_base64: Option<String>,
}


/// HashMap<Category, Vec<LaunchItem>>
pub type LaunchConfig = HashMap<String, Vec<LaunchItem>>;


use std::sync::Mutex;
use tauri::State;

/// 全局唯一配置的类型别名，便于 State 注入
pub type SharedConfig = Mutex<LaunchConfig>;

/// 初始化全局配置（如 config.json 不存在则创建默认）
pub fn init_config(path: &str) -> anyhow::Result<LaunchConfig> {
    match load_config(path) {
        Ok(cfg) => Ok(cfg),
        Err(_) => {
            let cfg: LaunchConfig = HashMap::new();
            save_config(&cfg, path)?;
            Ok(cfg)
        }
    }
}

/// 保存全局配置
pub fn save_shared_config(config: &State<SharedConfig>, path: &str) -> anyhow::Result<()> {
    let cfg = config.lock().unwrap();
    save_config(&cfg, path)
}


/// 加载配置
pub fn load_config(path: &str) -> anyhow::Result<LaunchConfig> {
    let content = fs::read_to_string(path)?;
    let config: LaunchConfig = serde_json::from_str(&content)?;
    Ok(config)
}

/// 保存配置
pub fn save_config(cfg: &LaunchConfig, path: &str) -> anyhow::Result<()> {
    let content = serde_json::to_string_pretty(cfg)?;
    fs::write(path, content)?;
    Ok(())
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
