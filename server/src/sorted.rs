use std::collections::BTreeSet;
use std::collections::hash_map::{Entry, HashMap};
use std::hash::Hash;

#[derive(Debug)]
pub struct Sorted<T, C> {
    counters: HashMap<T, C>,
    values: BTreeSet<(C, T)>,
}

impl<T, C> Sorted<T, C> {
    pub const fn values(&self) -> &BTreeSet<(C, T)> {
        &self.values
    }
}

impl<T: Clone + Eq + Hash + Ord, C: Copy + Default + Ord> Sorted<T, C> {
    pub fn modify(&mut self, value: T, f: impl Fn(&mut C)) {
        let entry = self.counters.entry(value.clone());
        let mut entry = match entry {
            Entry::Occupied(o) => {
                self.values.remove(&(*o.get(), value.clone()));
                o
            }
            Entry::Vacant(v) => v.insert_entry(C::default()),
        };
        let counter = entry.get_mut();
        f(counter);
        if *counter == C::default() {
            entry.remove();
        } else {
            self.values.insert((*counter, value));
        }
    }
}

impl<T, C> Default for Sorted<T, C> {
    fn default() -> Self {
        Self { counters: HashMap::new(), values: BTreeSet::new() }
    }
}
