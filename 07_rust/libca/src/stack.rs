use std::collections::LinkedList;

trait Stack<T> {
    fn push(&mut self, val: T);

    fn pop(&mut self) -> Option<T>;
    fn top(&self) -> Option<&T>;
    fn is_empty(&self) -> bool;
    fn len(&self) -> usize;
    fn clear(&mut self);
}

impl<T> Stack<T> for Vec<T> {
    fn push(&mut self, val: T) {
        self.push(val);
    }

    fn pop(&mut self) -> Option<T> {
        if self.is_empty() {
            return None;
        }
        Some(self.remove(self.len() - 1))
    }

    fn top(&self) -> Option<&T> {
        self.get(self.len() - 1)
    }

    fn is_empty(&self) -> bool {
        self.is_empty()
    }

    fn len(&self) -> usize {
        self.len()
    }

    fn clear(&mut self) {
        self.clear();
    }
}

impl<T> Stack<T> for LinkedList<T> {
    fn push(&mut self, val: T) {
        self.push_back(val);
    }

    fn pop(&mut self) -> Option<T> {
        self.pop_back()
    }

    fn top(&self) -> Option<&T> {
        self.back()
    }

    fn is_empty(&self) -> bool {
        self.is_empty()
    }

    fn len(&self) -> usize {
        self.len()
    }

    fn clear(&mut self) {
        self.clear();
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_vec_stack() {
        let mut stack = Vec::new();
        stack.push(1);
        stack.push(2);
        assert_eq!(stack.pop(), Some(2));
        assert_eq!(stack.pop(), Some(1));
        assert_eq!(stack.pop(), None);
        assert_eq!(stack.top(), None);
        assert_eq!(stack.len(), 0);
    }

    #[test]
    fn test_linked_list_stack() {
        let mut stack = LinkedList::new();
        stack.push(1);
        stack.push(2);
        assert_eq!(stack.pop(), Some(2));
        assert_eq!(stack.pop(), Some(1));
        assert_eq!(stack.pop(), None);
        assert_eq!(stack.top(), None);
        assert_eq!(stack.len(), 0);
    }
}
