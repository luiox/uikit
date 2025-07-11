extern crate proc_macro;

use clap::Parser;
use log::{info, trace};

#[derive(Parser)]
#[command(name = "simple-server")]
struct Cli {
    config: Option<String>,
}

fn main() {
    simple_logger::init().unwrap();
    info!("test info");

    trace!(target: "simple-server", "Starting simple-server");


    // let cli = Cli::parse();
    // info!("Config file: {}", "f");
}
