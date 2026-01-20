use chrono::{DateTime, Local};
use std::path::Path;

/// 模板处理器
pub struct TemplateProcessor;

impl TemplateProcessor {
    /// 处理模板字符串，替换占位符
    pub fn process_template(template: &str, file_name: &str) -> String {
        let now: DateTime<Local> = Local::now();
        let year = now.format("%Y").to_string();
        let date = now.format("%Y-%m-%d").to_string();
        
        let define_str = Self::generate_define_string(file_name);
        let ifndef_block = Self::generate_ifndef_block(&define_str);
        
        template
            .replace("{FileName}", file_name)
            .replace("{Year}", &year)
            .replace("{Date}", &date)
            .replace("{ifndef}", &ifndef_block)
    }
    
    /// 根据文件名生成预处理器定义字符串
    fn generate_define_string(file_name: &str) -> String {
        let path = Path::new(file_name);
        let name_without_ext = path.file_stem()
            .and_then(|s| s.to_str())
            .unwrap_or(file_name);
        
        let mut result = String::from("_");
        
        for c in name_without_ext.chars() {
            if c.is_uppercase() {
                result.push('_');
                result.push(c);
            } else {
                result.push(c.to_uppercase().next().unwrap_or(c));
            }
        }
        
        if let Some(extension) = path.extension().and_then(|s| s.to_str()) {
            result.push('_');
            result.push_str(&extension.to_uppercase());
            result.push('_');
        }
        
        // 确保以双下划线开头
        if result.starts_with("_") && !result.starts_with("__") {
            result.insert(0, '_');
        }
        
        result
    }
    
    /// 生成 #ifndef 代码块
    fn generate_ifndef_block(define_str: &str) -> String {
        format!(
            "#ifndef {}\n#define {}\n\n\n#endif\n",
            define_str, define_str
        )
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_generate_define_string() {
        assert_eq!(
            TemplateProcessor::generate_define_string("test.h"),
            "__TEST_H_"
        );
        
        assert_eq!(
            TemplateProcessor::generate_define_string("MyFile.c"),
            "__MY_FILE_C_"
        );
        
        assert_eq!(
            TemplateProcessor::generate_define_string("simple"),
            "__SIMPLE"
        );
    }
    
    #[test]
    fn test_process_template() {
        let template = "/* File: {FileName}\n * Date: {Date}\n * Year: {Year}\n */\n{ifndef}";
        let result = TemplateProcessor::process_template(template, "test.h");
        
        assert!(result.contains("test.h"));
        assert!(result.contains("#ifndef __TEST_H_"));
        assert!(result.contains("#define __TEST_H_"));
    }
}