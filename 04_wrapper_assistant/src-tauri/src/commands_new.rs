use crate::config::{load_config, save_config as save_config_file, LaunchConfig, LaunchItem};
use crate::launcher::{extract_icon_base64, LaunchProgramRequest};
use image::{ImageBuffer, Rgba};
use base64::engine::general_purpose::STANDARD as BASE64;
use base64::Engine;
use serde::Deserialize;
use std::collections::HashMap;

const CONFIG_PATH: &str = "config.json";

/// 启动程序
#[tauri::command]
pub fn launch_program(req: LaunchProgramRequest) -> Result<(), String> {
    crate::launcher::launch_program_impl(req)
}

/// 获取所有启动项配置
#[tauri::command]
pub fn get_all_launch_items() -> Result<LaunchConfig, String> {
    load_config(CONFIG_PATH).map_err(|e| e.to_string())
}

/// 获取程序图标的base64编码
#[tauri::command]
pub fn get_icon_base64(path: String) -> Result<String, String> {
    match extract_icon_base64(&path) {
        Some(b64) => Ok(b64),
        None => Ok(empty_png_base64()),
    }
}

/// 保存配置文件
#[tauri::command]
pub fn save_config_command(new_config: LaunchConfig) -> Result<(), String> {
    save_config_file(&new_config, CONFIG_PATH).map_err(|e| e.to_string())
}

/// 新增启动项
#[tauri::command]
pub fn add_launch_item(category: String, mut item: LaunchItem) -> Result<(), String> {
    let mut config = load_config(CONFIG_PATH).map_err(|e| e.to_string())?;
    
    // 自动提取图标
    if item.icon_base64.is_none() && !item.target_path.is_empty() {
        if let Some(icon) = extract_icon_base64(&item.target_path) {
            item.icon_base64 = Some(icon);
        }
    }
    
    // 初始化使用次数
    item.count = 0;
    
    // 添加到对应分类
    config.entry(category).or_insert_with(Vec::new).push(item);
    
    save_config_file(&config, CONFIG_PATH).map_err(|e| e.to_string())?;
    Ok(())
}

/// 编辑启动项
#[tauri::command]
pub fn edit_launch_item(old_name: String, category: String, mut item: LaunchItem) -> Result<(), String> {
    let mut config = load_config(CONFIG_PATH).map_err(|e| e.to_string())?;
    
    if let Some(items) = config.get_mut(&category) {
        if let Some(pos) = items.iter().position(|x| x.name == old_name) {
            // 保持原有的使用次数
            let old_count = items[pos].count;
            
            // 如果路径改变了，重新提取图标
            if item.target_path != items[pos].target_path && item.icon_base64.is_none() {
                if let Some(icon) = extract_icon_base64(&item.target_path) {
                    item.icon_base64 = Some(icon);
                }
            }
            
            item.count = old_count;
            items[pos] = item;
            
            save_config_file(&config, CONFIG_PATH).map_err(|e| e.to_string())?;
            return Ok(());
        }
    }
    
    Err("未找到指定的启动项".to_string())
}

/// 删除启动项
#[tauri::command]
pub fn remove_launch_item(name: String, category: String) -> Result<(), String> {
    let mut config = load_config(CONFIG_PATH).map_err(|e| e.to_string())?;
    
    if let Some(items) = config.get_mut(&category) {
        let original_len = items.len();
        items.retain(|item| item.name != name);
        
        if items.len() < original_len {
            // 如果分类变空了，删除整个分类
            if items.is_empty() {
                config.remove(&category);
            }
            
            save_config_file(&config, CONFIG_PATH).map_err(|e| e.to_string())?;
            Ok(())
        } else {
            Err("未找到指定的启动项".to_string())
        }
    } else {
        Err("未找到指定的分类".to_string())
    }
}

/// 更新启动项使用次数
#[tauri::command]
pub fn update_usage_count(name: String, category: String) -> Result<(), String> {
    let mut config = load_config(CONFIG_PATH).map_err(|e| e.to_string())?;
    
    if let Some(items) = config.get_mut(&category) {
        if let Some(item) = items.iter_mut().find(|x| x.name == name) {
            item.count += 1;
            save_config_file(&config, CONFIG_PATH).map_err(|e| e.to_string())?;
            return Ok(());
        }
    }
    
    Err("未找到指定的启动项".to_string())
}

/// 扫描系统程序（Windows）
#[tauri::command]
pub fn scan_system_programs() -> Result<Vec<LaunchItem>, String> {
    let mut programs = Vec::new();
    
    #[cfg(target_os = "windows")]
    {
        scan_windows_programs(&mut programs)?;
    }
    
    #[cfg(not(target_os = "windows"))]
    {
        return Err("当前仅支持 Windows 系统程序扫描".to_string());
    }
    
    // 为每个程序提取图标（限制数量避免太慢）
    for (i, program) in programs.iter_mut().enumerate() {
        if i > 50 { break; } // 限制扫描前50个程序的图标
        
        if let Some(icon) = extract_icon_base64(&program.target_path) {
            program.icon_base64 = Some(icon);
        }
    }
    
    Ok(programs)
}

#[cfg(target_os = "windows")]
fn scan_windows_programs(programs: &mut Vec<LaunchItem>) -> Result<(), String> {
    use std::fs;
    use std::path::Path;
    
    // 定义扫描路径
    let scan_paths = vec![
        "C:/Program Files",
        "C:/Program Files (x86)",
    ];
    
    // 常见的可执行文件名模式（避免扫描所有exe）
    let common_programs = vec![
        "chrome.exe", "firefox.exe", "edge.exe",
        "notepad.exe", "calc.exe", "mspaint.exe",
        "code.exe", "devenv.exe", "idea64.exe",
        "winrar.exe", "7zFM.exe",
        "explorer.exe", "cmd.exe", "powershell.exe",
    ];
    
    for base_path in scan_paths {
        if let Ok(entries) = fs::read_dir(base_path) {
            for entry in entries.flatten() {
                if entry.path().is_dir() {
                    // 扫描程序目录下的常见exe文件
                    scan_program_directory(&entry.path(), programs, &common_programs);
                }
            }
        }
    }
    
    // 扫描桌面快捷方式
    if let Some(desktop_path) = get_desktop_path() {
        scan_desktop_shortcuts(&desktop_path, programs);
    }
    
    Ok(())
}

#[cfg(target_os = "windows")]
fn scan_program_directory(dir_path: &Path, programs: &mut Vec<LaunchItem>, common_programs: &[&str]) {
    use std::fs;
    
    if let Ok(entries) = fs::read_dir(dir_path) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_file() {
                if let Some(file_name) = path.file_name().and_then(|n| n.to_str()) {
                    // 检查是否是常见程序或者exe文件
                    if common_programs.contains(&file_name.to_lowercase().as_str()) ||
                       file_name.to_lowercase().ends_with(".exe") {
                        
                        let name = path.file_stem()
                            .and_then(|s| s.to_str())
                            .unwrap_or("Unknown")
                            .to_string();
                            
                        programs.push(LaunchItem {
                            name,
                            target_path: path.to_string_lossy().to_string(),
                            icon_location: String::new(),
                            arguments: String::new(),
                            count: 0,
                            icon_base64: None,
                        });
                    }
                }
            }
        }
    }
}

#[cfg(target_os = "windows")]
fn get_desktop_path() -> Option<std::path::PathBuf> {
    dirs::desktop_dir()
}

#[cfg(target_os = "windows")]
fn scan_desktop_shortcuts(desktop_path: &Path, programs: &mut Vec<LaunchItem>) {
    use std::fs;
    
    if let Ok(entries) = fs::read_dir(desktop_path) {
        for entry in entries.flatten() {
            let path = entry.path();
            if let Some(ext) = path.extension() {
                if ext == "lnk" {
                    let name = path.file_stem()
                        .and_then(|s| s.to_str())
                        .unwrap_or("Unknown")
                        .to_string();
                        
                    programs.push(LaunchItem {
                        name,
                        target_path: path.to_string_lossy().to_string(),
                        icon_location: String::new(),
                        arguments: String::new(),
                        count: 0,
                        icon_base64: None,
                    });
                }
            }
        }
    }
}

// 生成32x32透明png的base64
fn empty_png_base64() -> String {
    let img = ImageBuffer::<Rgba<u8>, _>::from_pixel(32, 32, Rgba([0, 0, 0, 0]));
    let mut png_bytes = std::io::Cursor::new(Vec::new());
    let _ = image::DynamicImage::ImageRgba8(img).write_to(&mut png_bytes, image::ImageFormat::Png);
    let b64 = BASE64.encode(&png_bytes.into_inner());
    format!("data:image/png;base64,{}", b64)
}
