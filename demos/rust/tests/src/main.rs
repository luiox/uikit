use std::fs::File;
use std::io::{ErrorKind, Read};
use std::ops::{Add, Mul, Sub};
use std::process::Output;
use std::ptr::read;

enum Direction {
    Up = 0,
    Down,
    Left,
    Right,
}

#[derive(Debug)]
struct Point {
    x: i32,
    y: i32,
}

struct KeyVal(i32);

enum Event {
    KeyPress(KeyVal),
    MousePress(Point),
    MouseRelease(Point),
    MouseMove(Point),
}

fn apply(v: i32, f: fn(i32) -> i32) -> i32 {
    f(v)
}

fn cube(x: i32) -> i32 {
    x * x
}

struct Identifier();

fn sum1() -> i32 {
    let mut i = 1;
    let mut sum = 0;
    loop {
        sum += i;
        i += 1;

        if i > 100 {
            break;
        }
    }
    sum
}
fn sum2() -> i32 {
    let mut i = 1;
    let mut sum = 0;
    while i <= 100 {
        sum += i;
        i += 1;
    }
    sum
}

fn sum3() -> i32 {
    let mut sum = 0;
    for i in 1..=100 {
        sum += i;
    }
    sum
}

fn process_event(e: Event) -> i32 {
    match e {
        Event::KeyPress(KeyVal(v)) => {
            println!("KeyPress: {}", v);
            1
        }
        Event::MousePress(Point { x, y }) => {
            println!("MousePress: ({}, {})", x, y);
            2
        }
        Event::MouseRelease(Point { x, y }) => {
            println!("MouseRelease: ({}, {})", x, y);
            3
        }
        Event::MouseMove(Point { x, y }) => {
            print!("MouseMove:({}, {})", x, y);
            4
        }
    }
}

fn move_owner(s: String) {
    println!("{}", s);
}

fn borrow_val(s: &String) {
    println!("{}", s);
}

fn strcat(dest: &mut String, src: &String) {
    dest.push_str(src);
}
struct Id();

impl Point {
    fn new(x: i32, y: i32) -> Point {
        Point { x, y }
    }

    fn distance_to(&self, other: &Point) -> i32 {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        dx * dx + dy * dy
    }
}

trait MyFmtStr {
    fn to_my_fmt_str(&self) -> String;
}

impl MyFmtStr for i32 {
    fn to_my_fmt_str(&self) -> String {
        format!("num = {}", self)
    }
}

struct Point2(i32, i32);

struct MyBox<T> {
    value: T,
}
impl<T> MyBox<T> {
    fn new(value: T) -> MyBox<T> {
        MyBox { value }
    }
}
impl<T> std::ops::Deref for MyBox<T> {
    type Target = T;

    fn deref(&self) -> &Self::Target {
        &self.value
    }
}
impl<T> std::ops::Drop for MyBox<T> {
    fn drop(&mut self) {
        println!("drop");
    }
}

fn read_config_info(file_path: &str) -> Result<String, std::io::Error> {
    let mut buffer = String::new();
    File::open(file_path)?.read_to_string(&mut buffer)?;
    Ok(buffer)
}

fn print_number(maybe_number: Option<i32>) {
    match maybe_number {
        Some(number) => println!("Number: {}", number),
        None => println!("No number provided"),
    }
}

struct GPoint<T> {
    x: T,
    y: T,
}

trait Compare {
    fn compare(&self, other: &Self) -> i8;
}

fn max<T: Compare>(a: T, b: T) -> T {
    if a.compare(&b) > 0 { a } else { b }
}

impl<T> GPoint<T>
where
    T: Sub<Output = T> + Mul<Output = T> + Copy + PartialOrd + From<u8>,
{
    fn distance_to(&self, other: &GPoint<T>) -> T
    where
        T: std::marker::Copy,
    {
        let dx = self.x - other.x;
        let dy = self.y - other.y;
        let dx_squared = dx * dx;
        let dy_squared = dy * dy;
        dx_squared
        // let distance_squared = dx_squared + dy_squared;
        //
        // // 假设 T 可以调用 sqrt 函数，例如 f32 或 f64
        // T::from(2).sqrt() * distance_squared.sqrt()
    }
}

fn test1() {
    let gp1 = GPoint { x: 1, y: 2 };
    let gp2 = GPoint { x: 1.0, y: 2.0 };

    // let a = 1;
    // let b = 2;
    // let c = max(a, b);

    let file_result = File::open("test.txt");
    match file_result {
        Ok(file) => println!("file opened"),
        Err(error) => match error.kind() {
            ErrorKind::NotFound => println!("file not found"),
            ErrorKind::PermissionDenied => println!("permission denied"),
            _ => println!("other file error"),
        },
    }

    let p2 = Point2(1, 2);
    println!("{}, {}", p2.0, p2.1);

    let heap_p = MyBox::new(p2);
    println!("{}, {}", heap_p.value.0, heap_p.1);
    (*heap_p).0;

    drop(heap_p);

    let p = Point::new(1, 2);
    let p2 = Point {
        x: p.x + 10,
        y: p.y,
    };
    println!("distance = {}", p.distance_to(&p2));
    println!("{:?}", p);

    let key = KeyVal(1);
    key.0;

    let id = Id {};
    let p2 = Point { x: p.x + 10, ..p };

    let n1 = 1;
    let n2: i64 = 2;
    const PI: f64 = 3.141592;
    static mut N4: i32 = 4;

    if n1 > n2 {
        println!("n1 > n2");
    } else {
        println!("n1 <= n2");
    }

    let t = (1, 2);
    println!("{}", apply(5, cube));

    println!("{}", sum2());

    let s = "s1".to_string();
    let a = -1i32.abs();

    println!("{}", 666.to_my_fmt_str());
}

fn main() {
    let arr = vec![1, 2, 3, 4, 5];
    for i in arr.iter() {
        println!("{}", i);
    }
    let sum: i32 = arr.iter().sum();
    println!("sum = {}", sum);
}
