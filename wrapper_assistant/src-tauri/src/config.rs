use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

#[derive(Serialize, Deserialize, Clone)]
pub struct LaunchItem {
    pub category: String,
    pub name: String,
    pub target_path: String,
    pub icon_location: String,
    pub arguments: String,
    pub count: u32,
    pub icon_base64: Option<String>,
}

#[derive(Serialize, Deserialize, Clone)]
pub struct LaunchConfig {
    pub categories: Vec<String>,
    pub items: Vec<LaunchItem>,
}

impl LaunchConfig {
    pub fn load(path: &str) -> anyhow::Result<Self> {
        let content = fs::read_to_string(path)?;
        let config: LaunchConfig = serde_json::from_str(&content)?;
        Ok(config)
    }
    pub fn save(&self, path: &str) -> anyhow::Result<()> {
        let content = serde_json::to_string_pretty(self)?;
        fs::write(path, content)?;
        Ok(())
    }
}

// 图标提取相关
pub fn extract_icon_base64(exe_path: &str) -> Option<String> {
    // 这里只做接口声明，具体实现见 lib.rs
    None
}
