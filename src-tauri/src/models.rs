use serde::{Deserialize, Serialize};
use std::collections::BTreeMap;

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LauncherData {
    pub version: u8,
    pub groups: Vec<Group>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Group {
    pub id: String,
    pub name: String,
    pub order: i32,
    pub items: Vec<LaunchItem>,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct LaunchItem {
    pub id: String,
    pub item_type: String,
    pub name: String,
    pub target_path: String,
    pub icon_location: String,
    pub arguments: String,
    pub launch_count: u64,
    pub enabled: bool,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct Settings {
    pub hotkey: String,
    pub execute_hide: bool,
    pub current_group: Option<String>,
    #[serde(default = "default_group_panel_width")]
    pub group_panel_width: f64,
    #[serde(default = "default_main_window_width")]
    pub main_window_width: f64,
    #[serde(default = "default_main_window_height")]
    pub main_window_height: f64,
}

impl Default for Settings {
    fn default() -> Self {
        Self {
            hotkey: "Alt+1".to_string(),
            execute_hide: true,
            current_group: None,
            group_panel_width: default_group_panel_width(),
            main_window_width: default_main_window_width(),
            main_window_height: default_main_window_height(),
        }
    }
}

pub fn default_group_panel_width() -> f64 {
    220.0
}

pub fn default_main_window_width() -> f64 {
    1040.0
}

pub fn default_main_window_height() -> f64 {
    700.0
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct LauncherState {
    pub groups: Vec<Group>,
    pub settings: Settings,
    pub item_icons: BTreeMap<String, String>,
}

#[derive(Debug, Clone, Deserialize)]
#[serde(rename_all = "camelCase")]
pub struct ItemInput {
    pub id: Option<String>,
    pub item_type: Option<String>,
    pub name: String,
    pub target_path: String,
    pub icon_location: String,
    pub arguments: String,
    pub enabled: Option<bool>,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct LaunchResult {
    pub ok: bool,
    pub message: String,
}

#[derive(Debug, Clone, Serialize)]
#[serde(rename_all = "camelCase")]
pub struct EditorContext {
    pub group_id: String,
    pub item: Option<LaunchItem>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct LegacyItem {
    #[serde(rename = "Name")]
    pub name: String,
    #[serde(rename = "TargetPath")]
    pub target_path: String,
    #[serde(rename = "IconLocation")]
    pub icon_location: String,
    #[serde(rename = "Arguments")]
    pub arguments: String,
    #[serde(rename = "Count")]
    pub count: Option<u64>,
}
