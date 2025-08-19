mod config;
pub use config::*;
use tauri::State;
use std::sync::Mutex;

const CONFIG_PATH: &str = "config.json";

#[tauri::command]
fn get_all_launch_items() -> Result<LaunchConfig, String> {
    LaunchConfig::load(CONFIG_PATH).map_err(|e| e.to_string())
}

#[tauri::command]
fn get_icon_base64(path: String) -> Result<String, String> {
    match config::extract_icon_base64(&path) {
        Some(b64) => Ok(b64),
        None => Err("Failed to extract icon".to_string()),
    }
}

#[tauri::command]
fn save_config(new_config: LaunchConfig) -> Result<(), String> {
    new_config.save(CONFIG_PATH).map_err(|e| e.to_string())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            get_all_launch_items,
            get_icon_base64,
            save_config
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
