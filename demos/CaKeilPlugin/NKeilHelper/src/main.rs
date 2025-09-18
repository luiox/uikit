mod encoding;
mod template;
mod file_ops;

use clap::{Parser, Subcommand};
use file_ops::FileOperations;
use anyhow::Result;

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
    let cli = Cli::parse();

    match cli.command {
        Commands::Reset => {
            println!("Reset the config");
            // TODO: Implement config reset functionality
            Ok(())
        }
        Commands::Info => {
            print_help();
            Ok(())
        }
        Commands::FileComment { file } => {
            println!("Adding file comment to: {}", file);
            FileOperations::insert_comments(&file)
        }
        Commands::FunctionComment => {
            println!("Function comment feature is not implemented yet");
            Ok(())
        }
        Commands::LicenseComment => {
            println!("License comment feature is not implemented yet");
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
    println!("NKeilHelper version: 0.1.0");
    println!("A Rust implementation of CaKeilPlugin");
    println!();
    println!("Usage examples:");
    println!("  nkeil-helper reset");
    println!("  nkeil-helper help");
    println!("  nkeil-helper file-comment --file <FILE_PATH>");
    println!("  nkeil-helper function-comment");
    println!("  nkeil-helper license-comment");
}
