use std::time::{Duration, SystemTimeError};

struct Timer{
    pub start : u32,
}
impl Timer {
    pub fn new() -> Timer {
        Timer {
            start: 0,
        }
    }
    pub fn start(&mut self) {
        self.start = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap()
            .as_secs() as u32;
    }
    pub fn stop(&self) -> Result<Duration, SystemTimeError> {
        std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
    }
    
    pub fn passed(&self) -> Result<Duration, SystemTimeError>{
        let now = std::time::SystemTime::now()
            .duration_since(std::time::UNIX_EPOCH)
            .unwrap();
        Ok(now - Duration::from_secs(self.start as u64))
    }
}


fn main() {

    let a = [1; 2];
    for i in a.iter() {
        println!("{}", i);
    }
    let a = [1, 2];
    for i in a.iter() {
        println!("{}", i);
    }

    let mut timer = Timer::new();
    timer.start();


    'a: loop {
        if timer.passed().unwrap().as_secs() > 5 {
            break 'a;
        }
    }
    
}
