use encoding_rs::{Encoding, UTF_8, UTF_16BE, UTF_16LE};
use std::fs::File;
use std::io::{BufReader, Read};
use anyhow::Result;

/// 文件编码检测工具
pub struct FileEncoding;

impl FileEncoding {
    /// 检测文件编码
    pub fn detect_file_encoding(file_path: &str) -> Result<&'static Encoding> {
        let file = File::open(file_path)?;
        let mut reader = BufReader::new(file);
        let mut buffer = Vec::new();
        reader.read_to_end(&mut buffer)?;
        
        Ok(Self::detect_encoding(&buffer))
    }
    
    /// 从字节数组检测编码
    fn detect_encoding(bytes: &[u8]) -> &'static Encoding {
        // 检查 BOM
        if bytes.len() >= 3 {
            // UTF-8 BOM: EF BB BF
            if bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF {
                return UTF_8;
            }
        }
        
        if bytes.len() >= 2 {
            // UTF-16 BE BOM: FE FF
            if bytes[0] == 0xFE && bytes[1] == 0xFF {
                return UTF_16BE;
            }
            // UTF-16 LE BOM: FF FE
            if bytes[0] == 0xFF && bytes[1] == 0xFE {
                return UTF_16LE;
            }
        }
        
        // 如果没有 BOM，尝试检测是否为 UTF-8
        if Self::is_utf8_bytes(bytes) {
            return UTF_8;
        }
        
        // 默认返回本地编码 (假设为 UTF-8)
        UTF_8
    }
    
    /// 检查字节序列是否为有效的UTF-8
    fn is_utf8_bytes(data: &[u8]) -> bool {
        let mut remaining = 1;
        
        for &byte in data {
            if remaining == 1 {
                if byte >= 128 {
                    // 计算多字节字符的长度
                    let mut temp = byte;
                    remaining = 0;
                    while (temp & 0x80) != 0 {
                        remaining += 1;
                        temp <<= 1;
                    }
                    
                    if remaining == 1 || remaining > 6 {
                        return false;
                    }
                }
            } else {
                // 检查后续字节是否符合UTF-8格式 (10xxxxxx)
                if (byte & 0xC0) != 0x80 {
                    return false;
                }
                remaining -= 1;
            }
        }
        
        remaining == 1
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_is_utf8_bytes() {
        let utf8_bytes = "Hello, 世界!".as_bytes();
        assert!(FileEncoding::is_utf8_bytes(utf8_bytes));
        
        let ascii_bytes = "Hello, World!".as_bytes();
        assert!(FileEncoding::is_utf8_bytes(ascii_bytes));
    }
    
    #[test]
    fn test_detect_encoding() {
        // UTF-8 BOM
        let utf8_bom = vec![0xEF, 0xBB, 0xBF, 0x48, 0x65, 0x6C, 0x6C, 0x6F];
        assert_eq!(FileEncoding::detect_encoding(&utf8_bom).name(), "UTF-8");
        
        // UTF-16 BE BOM
        let utf16_be = vec![0xFE, 0xFF, 0x00, 0x48];
        assert_eq!(FileEncoding::detect_encoding(&utf16_be).name(), "UTF-16BE");
        
        // UTF-16 LE BOM
        let utf16_le = vec![0xFF, 0xFE, 0x48, 0x00];
        assert_eq!(FileEncoding::detect_encoding(&utf16_le).name(), "UTF-16LE");
    }
}