use std::fmt::{Display, Error, Formatter, Result};
use std::ops::{AddAssign, SubAssign};
use std::time;

use jiff::fmt::StdFmtWrite;
use jiff::fmt::friendly::{FractionalUnit, SpanPrinter};

#[derive(Clone, Copy, Debug, Default, Eq, Ord, PartialEq, PartialOrd)]
pub struct Duration(pub time::Duration);

impl Display for Duration {
    fn fmt(&self, f: &mut Formatter) -> Result {
        SpanPrinter::new()
            .fractional(Some(FractionalUnit::Second))
            .precision(Some(0))
            .print_unsigned_duration(&self.0, StdFmtWrite(f))
            .map_err(|_| Error)
    }
}

impl AddAssign for Duration {
    fn add_assign(&mut self, rhs: Self) {
        self.0 += rhs.0;
    }
}

impl SubAssign for Duration {
    fn sub_assign(&mut self, rhs: Self) {
        self.0 -= rhs.0;
    }
}
