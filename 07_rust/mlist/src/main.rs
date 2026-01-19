use std::ptr::null_mut;

struct List<T> {
    head: *mut Node<T>,
    tail: *mut Node<T>,
    len: usize,
}

struct Node<T> {
    data: T,
    next: *mut Node<T>,
}

impl<T> Node<T> {
    fn new(data: T) -> Self {
        Node {
            data,
            next: null_mut(),
        }
    }
}

impl<T> List<T> {
    fn new() -> Self {
        List {
            head: null_mut(),
            tail: null_mut(),
            len: 0,
        }
    }
    fn add(&mut self, data: T) {
        let mut node = Node::new(data);
        unsafe {
            let p = &mut node as *mut Node<T>;
            if self.head.is_null() {
                self.head = p;
                self.tail = p;
            } else {
                // find the last node
                let mut cur = self.head;
                while !cur.is_null() {
                    cur = (*cur).next;
                }
                (*cur).next = p;
                self.tail = p;
            }
            self.len += 1;
        }
    }
}

fn main() {
    let mut list = List::new();
    list.add(1);
    list.add(2);
    // for i in list{
    //     println!("{}", i);
    // }
}
