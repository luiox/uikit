use std::fs;

fn main() {
    let url = "https://www.rust-lang.org/";
    let output = "rust.md";

    let body = reqwest::blocking::get(url).unwrap().text().unwrap();
    println!("body : {}", body);

    let md = html2md::parse_html(&body);

    fs::write(output, md.as_bytes()).unwrap()

}
