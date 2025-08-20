#![allow(unused_imports)]
#![allow(unused_variables)]
#![allow(dead_code)]

mod config;
pub use config::*;
mod commands;
mod launcher;
pub use launcher::*; 
   
#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_opener::init())
        .invoke_handler(tauri::generate_handler![
            commands::get_all_launch_items,
            commands::get_icon_base64,
            commands::save_config_command
        ])
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
