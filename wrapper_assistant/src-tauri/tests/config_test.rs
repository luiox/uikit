use wrapper_assistant_lib::{LaunchConfig, LaunchItem, extract_icon_base64};
use std::fs;

fn sample_config_path() -> &'static str {
    "test_config.json"
}

fn sample_config() -> LaunchConfig {
    LaunchConfig {
        categories: vec!["IDE".to_string(), "Embedded".to_string()],
        items: vec![
            LaunchItem {
                category: "IDE".to_string(),
                name: "VSCode".to_string(),
                target_path: "C:/Program Files/VSCode/Code.exe".to_string(),
                icon_location: "C:/Program Files/VSCode/Code.exe".to_string(),
                arguments: "".to_string(),
                count: 10,
                icon_base64: None,
            },
        ],
    }
}

#[test]
fn test_save_and_load_config() {
    let config = sample_config();
    let path = sample_config_path();
    config.save(path).unwrap();
    let loaded = LaunchConfig::load(path).unwrap();
    assert_eq!(loaded.categories, config.categories);
    assert_eq!(loaded.items.len(), 1);
    assert_eq!(loaded.items[0].name, "VSCode");
    // 清理
    let _ = fs::remove_file(path);
}

#[test]
fn test_extract_icon_base64_none() {
    // 目前未实现，返回None
    let result = extract_icon_base64("not_exist.exe");
    assert!(result.is_none());
}
