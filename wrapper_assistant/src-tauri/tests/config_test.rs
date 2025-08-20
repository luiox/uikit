use wrapper_assistant_lib::{LaunchConfig, LaunchItem, extract_icon_base64};
use std::fs;

fn sample_config_path() -> &'static str {
    "test_config.json"
}

fn sample_config() -> LaunchConfig {
    use std::collections::HashMap;
    let mut map = HashMap::new();
    map.insert(
        "IDE".to_string(),
        vec![
            LaunchItem {
                name: "VSCode".to_string(),
                target_path: "C:/Program Files/VSCode/Code.exe".to_string(),
                icon_location: "C:/Program Files/VSCode/Code.exe".to_string(),
                arguments: "".to_string(),
                count: 10,
                icon_base64: None,
            },
        ],
    );
    map.insert("Embedded".to_string(), vec![]);
    map
}

#[test]
fn test_save_and_load_config() {
    let config = sample_config();
    let path = sample_config_path();
    wrapper_assistant_lib::save_config(&config, path).unwrap();
    let loaded = wrapper_assistant_lib::load_config(path).unwrap();
    assert!(loaded.contains_key("IDE"));
    assert_eq!(loaded["IDE"].len(), 1);
    assert_eq!(loaded["IDE"][0].name, "VSCode");
    // 清理
    let _ = fs::remove_file(path);
}

#[test]
fn test_extract_icon_base64_none() {
    // 目前未实现，返回None
    let result = 
        extract_icon_base64(r#"D:\Program Files\Microsoft VS Code\Code.exe"#);
    println!("Extracted icon base64: {:?}", result);
    assert!(result.is_some());
}
