use crate::encoding::FileEncoding;
use crate::template::TemplateProcessor;
use encoding_rs::Encoding;
use std::fs;
use std::path::{Path, PathBuf};
use anyhow::{Context, Result, anyhow};

/// 文件操作工具
pub struct FileOperations;

impl FileOperations {
    /// 向文件插入注释
    pub fn insert_comments(file_path: &str) -> Result<()> {
        let file_path = Path::new(file_path);
        
        // 检查文件是否存在
        if !file_path.exists() {
            return Err(anyhow!("文件不存在: {}", file_path.display()));
        }
        
        let file_name = file_path.file_name()
            .and_then(|name| name.to_str())
            .ok_or_else(|| anyhow!("无法获取文件名"))?;
        
        // 根据文件扩展名选择模板
        let template_name = if file_path.extension()
            .and_then(|ext| ext.to_str())
            .map(|ext| ext.to_lowercase()) == Some("h".to_string()) {
            "File.h"
        } else {
            "File.c"
        };
        
        // 查找模板文件
        let template_path = Self::find_template_file(template_name)?;
        
        // 读取模板内容
        let encoding = FileEncoding::detect_file_encoding(
            template_path.to_str().unwrap()
        )?;
        let template_content = Self::read_file_with_encoding(&template_path, encoding)?;
        
        // 处理模板
        let processed_template = TemplateProcessor::process_template(&template_content, file_name);
        
        // 读取原文件内容
        let original_encoding = FileEncoding::detect_file_encoding(
            file_path.to_str().unwrap()
        )?;
        let original_content = Self::read_file_with_encoding(file_path, original_encoding)?;
        
        // 合并内容
        let new_content = format!("{}{}", processed_template, original_content);
        
        // 写回文件
        Self::write_file_with_encoding(file_path, &new_content, original_encoding)?;
        
        println!("成功为文件 '{}' 添加注释", file_name);
        Ok(())
    }
    
    /// 查找模板文件
    fn find_template_file(template_name: &str) -> Result<PathBuf> {
        // 首先在当前执行目录查找
        let current_dir = std::env::current_exe()
            .context("无法获取当前执行文件路径")?
            .parent()
            .ok_or_else(|| anyhow!("无法获取执行目录"))?
            .to_path_buf();
        
        let template_path = current_dir.join(template_name);
        if template_path.exists() {
            return Ok(template_path);
        }
        
        // 在当前工作目录查找
        let working_dir = std::env::current_dir()
            .context("无法获取当前工作目录")?;
        
        let template_path = working_dir.join(template_name);
        if template_path.exists() {
            return Ok(template_path);
        }
        
        Err(anyhow!("模板文件 '{}' 未找到", template_name))
    }
    
    /// 使用指定编码读取文件
    fn read_file_with_encoding(file_path: &Path, encoding: &'static Encoding) -> Result<String> {
        let bytes = fs::read(file_path)
            .with_context(|| format!("无法读取文件: {}", file_path.display()))?;
        
        let (content, _, had_errors) = encoding.decode(&bytes);
        
        if had_errors {
            eprintln!("警告: 解码文件时遇到错误: {}", file_path.display());
        }
        
        Ok(content.to_string())
    }
    
    /// 使用指定编码写入文件
    fn write_file_with_encoding(
        file_path: &Path, 
        content: &str, 
        encoding: &'static Encoding
    ) -> Result<()> {
        let (bytes, _, had_errors) = encoding.encode(content);
        
        if had_errors {
            eprintln!("警告: 编码文件时遇到错误: {}", file_path.display());
        }
        
        fs::write(file_path, &*bytes)
            .with_context(|| format!("无法写入文件: {}", file_path.display()))?;
        
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;
    use tempfile::tempdir;
    
    #[test]
    fn test_find_template_file() {
        let temp_dir = tempdir().unwrap();
        let template_path = temp_dir.path().join("File.h");
        fs::write(&template_path, "/* Template content */").unwrap();
        
        // 修改当前目录到临时目录
        let original_dir = std::env::current_dir().unwrap();
        std::env::set_current_dir(temp_dir.path()).unwrap();
        
        let found_path = FileOperations::find_template_file("File.h");
        
        // 恢复原始目录
        std::env::set_current_dir(original_dir).unwrap();
        
        assert!(found_path.is_ok());
    }
}