use hello_macro::AccessorGen;

#[macro_export]
macro_rules! vec {
    ( $( $x:expr ),+ $(,)?) => {
        {
            let mut temp_vec = Vec::new();
            $(
                temp_vec.push($x);
            )*
            temp_vec
        }
    };
}

#[derive(AccessorGen)]
struct User {
    #[setter]
    #[getter]
    id: u32,
    #[setter]
    #[getter]
    name: String,
}

fn main() {
    let u1 = User {
        id: 1,
        name: "John".to_string(),
    };
    println!("{}", u1.name);

    // let vec = std::vec![1, 2, 3,];
    // for v in vec {
    //     println!("{}", v);
    // }
}
