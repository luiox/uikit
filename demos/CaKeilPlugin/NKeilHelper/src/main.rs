mod encoding;
mod template;
mod file_ops;
mod i18n;

use clap::{Parser, Subcommand};
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
    Reset,
    /// Print detailed help information with logo
    Info,
    /// Add file comment to the specified file
    FileComment {
        /// Path to the file to add comments to
        #[arg(short, long)]
        file: String,
    },
    /// Add function comment (not implemented yet)
    FunctionComment,
    /// Add license comment (not implemented yet)
    LicenseComment,
}

fn main() -> Result<()> {
    // 初始化国际化
    init_i18n();
    
    let cli = Cli::parse();

    match cli.command {
        Commands::Reset => {
            println!("{}", translate("success.config_reset"));
            // TODO: Implement config reset functionality
            Ok(())
        }
        Commands::Info => {
            print_help();
            Ok(())
        }
        Commands::FileComment { file } => {
            println!("{}", translate_with_args("status.adding_comment", &[("file", &file)]));
            FileOperations::insert_comments(&file)
        }
        Commands::FunctionComment => {
            println!("{}", translate("status.not_implemented"));
            Ok(())
        }
        Commands::LicenseComment => {
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
}
