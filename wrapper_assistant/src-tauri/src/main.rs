#![cfg_attr(
    all(not(debug_assertions), target_os = "windows"),
    windows_subsystem = "windows"
)]

// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#[cfg_attr(mobile, tauri::mobile_entry_point)]
fn main() {
    vite_tauri::run()
}
