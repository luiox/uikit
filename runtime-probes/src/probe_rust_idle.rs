use std::thread;
use std::time::Duration;

fn main() {
    println!("probe-rust-idle started");
    loop {
        thread::sleep(Duration::from_secs(1));
    }
}
