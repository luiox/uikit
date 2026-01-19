use rand::Rng;
use std::cmp::Ordering;
use std::io;
fn main() {
    let num = rand::thread_rng().gen_range(1..100);

    println!("{}", num);

    loop {
        let mut input = String::new();
        io::stdin()
            .read_line(&mut input)
            .expect("Failed to read line");
        let input: u32 = match input.trim().parse() {
            Ok(num) => num,
            Err(_) => continue,
        };

        match input.cmp(&num) {
            Ordering::Less => println!("Too low"),
            Ordering::Greater => println!("Too high"),
            Ordering::Equal => {
                println!("Correct");
                break;
            }
        }
    }

}
