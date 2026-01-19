use std::collections::LinkedList;

trait Queue<T>{
    fn add(&mut self,t:T);
    fn poll(&mut self)->T;
    fn peek(&self)->&T;
    fn is_empty(&self)->bool;
    fn size(&self)->usize;
    fn clear(&mut self);
}

impl<T> Queue<T> for LinkedList<T>{
    fn add(&mut self, t:T){
        self.push_back(t);
    }

    fn poll(&mut self) -> T {
        if self.is_empty() {
            panic!("queue is empty");
        }
        self.pop_front().unwrap()
    }

    fn peek(&self) -> &T {
        self.front().unwrap()
    }

    fn is_empty(&self) -> bool {
        self.is_empty()
    }

    fn size(&self) -> usize {
        self.len()
    }

    fn clear(&mut self) {
        self.clear()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn test_queue() {
        let mut q = LinkedList::new();
        q.add(1);
        q.add(2);
        q.add(3);
        assert_eq!(q.size(), 3);
        assert_eq!(q.poll(), 1);
        assert_eq!(*q.peek(), 2);
        assert_eq!(q.size(), 2);
        assert_eq!(q.is_empty(), false);
        q.clear();
        assert_eq!(q.is_empty(), true);
    }
}