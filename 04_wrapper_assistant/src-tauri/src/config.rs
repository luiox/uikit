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


/// 加载配置（如果文件不存在则返回空配置）
pub fn load_config_or_create(path: &str) -> anyhow::Result<LaunchConfig> {
    match load_config(path) {
        Ok(cfg) => Ok(cfg),
        Err(_) => {
            // 文件不存在，创建空配置
            let cfg: LaunchConfig = HashMap::new();
            save_config(&cfg, path)?;
            Ok(cfg)
        }
    }
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
