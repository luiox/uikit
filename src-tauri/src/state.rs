use std::{
    collections::HashMap,
    path::PathBuf,
    sync::Mutex,
};
use tauri::tray::TrayIcon;

use crate::models::{EditorContext, LauncherData, Settings};

pub struct AppState {
    pub data: Mutex<LauncherData>,
    pub settings: Mutex<Settings>,
    pub editor_context: Mutex<Option<EditorContext>>,
    pub tray_icon: Mutex<Option<TrayIcon>>,
    pub icon_cache: Mutex<HashMap<String, String>>,
    pub icons_dir: PathBuf,
    pub icons_index_path: PathBuf,
    pub data_path: PathBuf,
    pub settings_path: PathBuf,
}

impl AppState {
    pub fn new(
        data: LauncherData,
        settings: Settings,
        icons_dir: PathBuf,
        icons_index_path: PathBuf,
        data_path: PathBuf,
        settings_path: PathBuf,
    ) -> Self {
        Self {
            data: Mutex::new(data),
            settings: Mutex::new(settings),
            editor_context: Mutex::new(None),
            tray_icon: Mutex::new(None),
            icon_cache: Mutex::new(HashMap::new()),
            icons_dir,
            icons_index_path,
            data_path,
            settings_path,
        }
    }

    pub fn save_data(&self) -> Result<(), String> {
        let data = self.data.lock().map_err(|_| "state lock poisoned".to_string())?;
        crate::write_json_atomic(&self.data_path, &*data)
    }

    pub fn save_settings(&self) -> Result<(), String> {
        let settings = self
            .settings
            .lock()
            .map_err(|_| "settings lock poisoned".to_string())?;
        crate::write_json_atomic(&self.settings_path, &*settings)
    }
}
