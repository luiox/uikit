use crate::config::{LaunchConfig, extract_icon_base64};

const CONFIG_PATH: &str = "config.json";

#[tauri::command]
pub fn get_all_launch_items() -> Result<LaunchConfig, String> {
    LaunchConfig::load(CONFIG_PATH).map_err(|e| e.to_string())
}

#[tauri::command]
pub fn get_icon_base64(path: String) -> Result<String, String> {
    match extract_icon_base64(&path) {
        Some(b64) => Ok(b64),
        None => Err("Failed to extract icon".to_string()),
    }
}

#[tauri::command]
pub fn save_config(new_config: LaunchConfig) -> Result<(), String> {
    new_config.save(CONFIG_PATH).map_err(|e| e.to_string())
}
