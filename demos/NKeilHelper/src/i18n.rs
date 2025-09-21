use std::collections::HashMap;
use std::env;
use once_cell::sync::Lazy;

/// 静态翻译数据
static TRANSLATIONS: Lazy<HashMap<String, HashMap<String, String>>> = Lazy::new(|| {
    let mut translations = HashMap::new();
    
    // 英语翻译
    let mut en = HashMap::new();
    en.insert("app.name".to_string(), "NKeilHelper".to_string());
    en.insert("app.version".to_string(), "Version: 0.1.0".to_string());
    en.insert("app.copyright".to_string(), "Rust version of CaKeilPlugin".to_string());
    en.insert("success.config_reset".to_string(), "Reset the config".to_string());
    en.insert("success.comment_added".to_string(), "Successfully added comment to file '%{file}'".to_string());
    en.insert("status.adding_comment".to_string(), "Adding file comment to: %{file}".to_string());
    en.insert("status.not_implemented".to_string(), "Feature is not implemented yet".to_string());
    en.insert("error.file_not_found".to_string(), "File does not exist: %{file}".to_string());
    en.insert("error.template_not_found".to_string(), "Template file '%{template}' not found".to_string());
    en.insert("error.cannot_get_filename".to_string(), "Cannot get filename".to_string());
    en.insert("error.cannot_read_file".to_string(), "Cannot read file: %{file}".to_string());
    en.insert("error.cannot_write_file".to_string(), "Cannot write file: %{file}".to_string());
    en.insert("error.encoding_error".to_string(), "Warning: Error occurred while decoding file: %{file}".to_string());
    en.insert("error.encoding_write_error".to_string(), "Warning: Error occurred while encoding file: %{file}".to_string());
    en.insert("error.cannot_get_current_dir".to_string(), "Cannot get current working directory".to_string());
    en.insert("error.cannot_get_exe_path".to_string(), "Cannot get current executable path".to_string());
    en.insert("error.cannot_get_exe_dir".to_string(), "Cannot get executable directory".to_string());
    en.insert("help.usage_examples".to_string(), "Usage examples".to_string());
    en.insert("help.example_reset".to_string(), "nkeil-helper reset".to_string());
    en.insert("help.example_info".to_string(), "nkeil-helper info".to_string());
    en.insert("help.example_file_comment".to_string(), "nkeil-helper file-comment --file <FILE_PATH>".to_string());
    en.insert("help.example_function_comment".to_string(), "nkeil-helper function-comment".to_string());
    en.insert("help.example_license_comment".to_string(), "nkeil-helper license-comment".to_string());
    en.insert("help.example_hide_window".to_string(), "nkeil-helper --show=false file-comment --file <FILE_PATH>".to_string());
    en.insert("args.show".to_string(), "Show or hide console window (Windows only)".to_string());
    
    // 中文翻译
    let mut zh = HashMap::new();
    zh.insert("app.name".to_string(), "NKeilHelper".to_string());
    zh.insert("app.version".to_string(), "版本: 0.1.0".to_string());
    zh.insert("app.copyright".to_string(), "Rust 版本的 CaKeilPlugin".to_string());
    zh.insert("success.config_reset".to_string(), "重置配置".to_string());
    zh.insert("success.comment_added".to_string(), "成功为文件 '%{file}' 添加注释".to_string());
    zh.insert("status.adding_comment".to_string(), "正在为文件添加注释: %{file}".to_string());
    zh.insert("status.not_implemented".to_string(), "功能尚未实现".to_string());
    zh.insert("error.file_not_found".to_string(), "文件不存在: %{file}".to_string());
    zh.insert("error.template_not_found".to_string(), "模板文件 '%{template}' 未找到".to_string());
    zh.insert("error.cannot_get_filename".to_string(), "无法获取文件名".to_string());
    zh.insert("error.cannot_read_file".to_string(), "无法读取文件: %{file}".to_string());
    zh.insert("error.cannot_write_file".to_string(), "无法写入文件: %{file}".to_string());
    zh.insert("error.encoding_error".to_string(), "警告: 解码文件时遇到错误: %{file}".to_string());
    zh.insert("error.encoding_write_error".to_string(), "警告: 编码文件时遇到错误: %{file}".to_string());
    zh.insert("error.cannot_get_current_dir".to_string(), "无法获取当前工作目录".to_string());
    zh.insert("error.cannot_get_exe_path".to_string(), "无法获取当前执行文件路径".to_string());
    zh.insert("error.cannot_get_exe_dir".to_string(), "无法获取执行目录".to_string());
    zh.insert("help.usage_examples".to_string(), "使用示例".to_string());
    zh.insert("help.example_reset".to_string(), "nkeil-helper reset".to_string());
    zh.insert("help.example_info".to_string(), "nkeil-helper info".to_string());
    zh.insert("help.example_file_comment".to_string(), "nkeil-helper file-comment --file <文件路径>".to_string());
    zh.insert("help.example_function_comment".to_string(), "nkeil-helper function-comment".to_string());
    zh.insert("help.example_license_comment".to_string(), "nkeil-helper license-comment".to_string());
    zh.insert("help.example_hide_window".to_string(), "nkeil-helper --show=false file-comment --file <文件路径>".to_string());
    zh.insert("args.show".to_string(), "显示或隐藏控制台窗口（仅限 Windows）".to_string());
    
    translations.insert("en".to_string(), en);
    translations.insert("zh-CN".to_string(), zh);
    
    translations
});

/// 全局语言设置
static LANGUAGE: Lazy<String> = Lazy::new(|| {
    // 从环境变量获取语言设置
    if let Ok(lang) = env::var("NKEIL_LANG") {
        return normalize_locale(&lang);
    }
    
    if let Ok(lang) = env::var("LANG") {
        return normalize_locale(&lang);
    }
    
    // Windows 系统语言检测
    #[cfg(windows)]
    {
        if let Some(lang) = get_windows_locale() {
            return normalize_locale(&lang);
        }
    }
    
    // 默认英语
    "en".to_string()
});

/// 标准化语言代码
fn normalize_locale(locale: &str) -> String {
    let locale = locale.to_lowercase();
    
    if locale.starts_with("zh") {
        return "zh-CN".to_string();
    }
    
    if locale.starts_with("en") {
        return "en".to_string();
    }
    
    "en".to_string()
}

/// Windows 系统语言检测
#[cfg(windows)]
fn get_windows_locale() -> Option<String> {
    use std::process::Command;
    
    if let Ok(output) = Command::new("powershell")
        .args(&["-Command", "Get-Culture | Select-Object -ExpandProperty Name"])
        .output()
    {
        if let Ok(locale) = String::from_utf8(output.stdout) {
            return Some(locale.trim().to_string());
        }
    }
    
    None
}

/// 初始化国际化系统
pub fn init_i18n() {
    // 预加载翻译数据
    Lazy::force(&TRANSLATIONS);
    Lazy::force(&LANGUAGE);
}

/// 获取当前语言
pub fn get_current_language() -> &'static str {
    &LANGUAGE
}

/// 翻译函数
pub fn translate(key: &str) -> String {
    let lang = &*LANGUAGE;
    
    if let Some(lang_map) = TRANSLATIONS.get(lang) {
        if let Some(text) = lang_map.get(key) {
            return text.clone();
        }
    }
    
    // 回退到英语
    if let Some(en_map) = TRANSLATIONS.get("en") {
        if let Some(text) = en_map.get(key) {
            return text.clone();
        }
    }
    
    // 如果都找不到，返回键值
    key.to_string()
}

/// 带参数的翻译函数
pub fn translate_with_args(key: &str, args: &[(&str, &str)]) -> String {
    let mut result = translate(key);
    
    for (placeholder, value) in args {
        let placeholder_pattern = format!("%{{{}}}", placeholder);
        result = result.replace(&placeholder_pattern, value);
    }
    
    result
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_normalize_locale() {
        assert_eq!(normalize_locale("zh-CN"), "zh-CN");
        assert_eq!(normalize_locale("zh_CN"), "zh-CN");
        assert_eq!(normalize_locale("zh"), "zh-CN");
        assert_eq!(normalize_locale("en-US"), "en");
        assert_eq!(normalize_locale("en"), "en");
        assert_eq!(normalize_locale("fr"), "en"); // 不支持的语言回退到英语
    }
    
    #[test]
    fn test_translation() {
        init_i18n();
        
        // 测试基本翻译
        let app_name = translate("app.name");
        assert_eq!(app_name, "NKeilHelper");
        
        // 测试带参数的翻译
        let msg = translate_with_args("success.comment_added", &[("file", "test.c")]);
        assert!(msg.contains("test.c"));
    }
}