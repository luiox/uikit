use serde::Deserialize;
use crate::launcher::LaunchProgramRequest;

/// 启动程序
/// 参数: target_path - 可执行文件路径
///      arguments - 命令行参数（可选）
///      run_as_admin - 是否以管理员权限运行
/// 返回: Result<(), String>
#[tauri::command]
pub fn launch_program(req: LaunchProgramRequest) -> Result<(), String> {
    crate::launcher::launch_program_impl(req)
}
use crate::config::{LaunchConfig, extract_icon_base64, load_config, save_config as save_config_file};

const CONFIG_PATH: &str = "config.json";

#[tauri::command]
pub fn get_all_launch_items() -> Result<LaunchConfig, String> {
    load_config(CONFIG_PATH).map_err(|e| e.to_string())
}

#[tauri::command]
pub fn get_icon_base64(path: String) -> Result<String, String> {
    match extract_icon_base64(&path) {
        Some(b64) => Ok(b64),
        None => Err("Failed to extract icon".to_string()),
    }
}

#[tauri::command]
pub fn save_config_command(new_config: LaunchConfig) -> Result<(), String> {
    save_config_file(&new_config, CONFIG_PATH).map_err(|e| e.to_string())
}

/// 新增启动项
/// 参数: item - 启动项对象，包含 category、name、target_path、icon_location、arguments 等字段
/// 返回: Result<(), String> 或返回最新配置数据
#[tauri::command]
pub fn add_launch_item(/* item: LaunchItem */) -> Result<(), String> {
    // TODO: 实现新增启动项逻辑
    unimplemented!()
}

/// 编辑启动项
/// 参数: old_name - 原启动项名称
///      item - 新的启动项对象
/// 返回: Result<(), String> 或返回最新配置数据
#[tauri::command]
pub fn edit_launch_item(/* old_name: String, item: LaunchItem */) -> Result<(), String> {
    // TODO: 实现编辑启动项逻辑
    unimplemented!()
}

/// 删除启动项
/// 参数: name - 启动项名称
///      category - 启动项分类
/// 返回: Result<(), String> 或返回最新配置数据
#[tauri::command]
pub fn remove_launch_item(/* name: String, category: String */) -> Result<(), String> {
    // TODO: 实现删除启动项逻辑
    unimplemented!()
}

/// 获取完整配置文件内容
/// 返回: config.json 的全部内容
#[tauri::command]
pub fn get_config() -> Result<LaunchConfig, String> {
    // TODO: 实现获取配置文件内容逻辑
    unimplemented!()
}
