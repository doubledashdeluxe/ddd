use std::fmt::{Display, Formatter, Result};
use std::ops::{Add, Div};

#[derive(Clone, Copy, Debug)]
pub struct Counters {
    pub tx: u64,
    pub rx: u64,
    pub ops: u64,
}

impl Counters {
    pub const fn new() -> Self {
        Self { tx: 0, rx: 0, ops: 0 }
    }
}

impl Display for Counters {
    fn fmt(&self, f: &mut Formatter) -> Result {
        write!(f, "{:>8} {:>8} {:>10}", self.tx, self.rx, self.ops)
    }
}

impl Default for Counters {
    fn default() -> Self {
        Self::new()
    }
}

impl Add for Counters {
    type Output = Self;

    fn add(self, rhs: Self) -> Self {
        Self { tx: self.tx + rhs.tx, rx: self.rx + rhs.rx, ops: self.ops + rhs.ops }
    }
}

impl Div<u64> for Counters {
    type Output = Self;

    fn div(self, rhs: u64) -> Self {
        Self { tx: self.tx / rhs, rx: self.rx / rhs, ops: self.ops / rhs }
    }
}
