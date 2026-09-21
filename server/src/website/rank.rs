use std::fmt::{Display, Formatter, Result};

#[derive(Clone, Copy, Debug)]
pub struct Rank(pub usize);

impl Display for Rank {
    fn fmt(&self, f: &mut Formatter) -> Result {
        let suffix = match self.0 % 10 {
            0 => "st",
            1 => "nd",
            2 => "rd",
            _ => "th",
        };
        write!(f, "{}{suffix}", self.0 + 1)
    }
}
