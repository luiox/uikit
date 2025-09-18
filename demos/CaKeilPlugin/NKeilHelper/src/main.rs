mod encoding;
mod template;
mod file_ops;
mod i18n;

use clap::{Parser, Subcommand, ArgAction};
use file_ops::FileOperations;
use anyhow::Result;
use i18n::{init_i18n, translate, translate_with_args};

/// NKeilHelper - A Keil helper tool for file comment insertion and template processing
#[derive(Parser)]
#[command(name = "nkeil-helper")]
#[command(about = "A Keil helper tool for file comment insertion and template processing")]
#[command(version = "0.1.0")]
#[command(long_about = r#"
  _____           _  __         _   _   _____    _                   _         
 / ____|         | |/ /        (_) | | |  __ \  | |                 (_)        
| |        __ _  | ' /    ___   _  | | | |__) | | |  _   _    __ _   _   _ __  
| |       / _` | |  <    / _ \ | | | | |  ___/  | | | | | |  / _` | | | | '_ \ 
| |____  | (_| | | . \  |  __/ | | | | | |      | | | |_| | | (_| | | | | | | |
 \_____|  \__,_| |_|\_\  \___| |_| |_| |_|      |_|  \__,_|  \__, | |_| |_| |_|
                                                              __/ |            
                                                             |___/             

NKeilHelper - Rust version of CaKeilPlugin
"#)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Reset the configuration
    Reset {
        /// Show or hide console window (Windows only)
        #[arg(long, default_value = "true", value_name = "BOOL", action = clap::ArgAction::Set)]
        show: bool,
    },
    /// Print detailed help information with logo
    Info {
        /// Show or hide console window (Windows only)
        #[arg(long, default_value = "true", value_name = "BOOL", action = clap::ArgAction::Set)]
        show: bool,
    },
    /// Add file comment to the specified file
    FileComment {
        /// Path to the file to add comments to
        #[arg(short, long)]
        file: String,
        /// Show or hide console window (Windows only)
        #[arg(long, default_value = "true", value_name = "BOOL", action = clap::ArgAction::Set)]
        show: bool,
    },
    /// Add function comment (not implemented yet)
    FunctionComment {
        /// Show or hide console window (Windows only)
        #[arg(long, default_value = "true", value_name = "BOOL", action = clap::ArgAction::Set)]
        show: bool,
    },
    /// Add license comment (not implemented yet)
    LicenseComment {
        /// Show or hide console window (Windows only)
        #[arg(long, default_value = "true", value_name = "BOOL", action = clap::ArgAction::Set)]
        show: bool,
    },
}

// Windows API 声明
#[cfg(windows)]
mod windows_api {
    use std::ffi::c_void;
    
    #[link(name = "kernel32")]
    extern "system" {
        pub fn GetConsoleWindow() -> *mut c_void;
    }
    
    #[link(name = "user32")]
    extern "system" {
        pub fn ShowWindow(hwnd: *mut c_void, cmd_show: i32) -> i32;
    }
    
    pub const SW_HIDE: i32 = 0;
    pub const SW_SHOW: i32 = 5;
}

/// 设置控制台窗口显示状态（仅在 Windows 上有效）
#[cfg(windows)]
fn set_console_visibility(show: bool) {
    unsafe {
        let console_window = windows_api::GetConsoleWindow();
        if !console_window.is_null() {
            let cmd = if show { windows_api::SW_SHOW } else { windows_api::SW_HIDE };
            windows_api::ShowWindow(console_window, cmd);
        }
    }
}

/// 在非 Windows 系统上的空实现
#[cfg(not(windows))]
fn set_console_visibility(_show: bool) {
    // 在非 Windows 系统上不执行任何操作
}

fn main() -> Result<()> {
    // 初始化国际化
    init_i18n();
    
    let cli = Cli::parse();

    match cli.command {
        Commands::Reset { show } => {
            // 设置控制台窗口显示状态
            set_console_visibility(show);
            println!("{}", translate("success.config_reset"));
            // TODO: Implement config reset functionality
            Ok(())
        }
        Commands::Info { show } => {
            // 设置控制台窗口显示状态
            set_console_visibility(show);
            print_help();
            Ok(())
        }
        Commands::FileComment { file, show } => {
            // 设置控制台窗口显示状态
            set_console_visibility(show);
            println!("{}", translate_with_args("status.adding_comment", &[("file", &file)]));
            FileOperations::insert_comments(&file)
        }
        Commands::FunctionComment { show } => {
            // 设置控制台窗口显示状态
            set_console_visibility(show);
            println!("{}", translate("status.not_implemented"));
            Ok(())
        }
        Commands::LicenseComment { show } => {
            // 设置控制台窗口显示状态
            set_console_visibility(show);
            println!("{}", translate("status.not_implemented"));
            Ok(())
        }
    }
}

fn print_help() {
    let logo = r#"
  _____           _  __         _   _   _____    _                   _         
 / ____|         | |/ /        (_) | | |  __ \  | |                 (_)        
| |        __ _  | ' /    ___   _  | | | |__) | | |  _   _    __ _   _   _ __  
| |       / _` | |  <    / _ \ | | | | |  ___/  | | | | | |  / _` | | | | '_ \ 
| |____  | (_| | | . \  |  __/ | | | | | |      | | | |_| | | (_| | | | | | | |
 \_____|  \__,_| |_|\_\  \___| |_| |_| |_|      |_|  \__,_|  \__, | |_| |_| |_|
                                                              __/ |            
                                                             |___/             
"#;
    
    println!("{}", logo);
    println!("{}", translate("app.name"));
    println!("{}", translate("app.version"));
    println!("{}", translate("app.copyright"));
    println!();
    println!("{}:", translate("help.usage_examples"));
    println!("  {}", translate("help.example_reset"));
    println!("  {}", translate("help.example_info"));
    println!("  {}", translate("help.example_file_comment"));
    println!("  {}", translate("help.example_function_comment"));
    println!("  {}", translate("help.example_license_comment"));
    println!("  {}", translate("help.example_hide_window"));
}
