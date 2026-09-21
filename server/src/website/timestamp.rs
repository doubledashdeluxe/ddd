use std::fmt::{self, Display};

use jiff::Timestamp;
use jiff::tz::TimeZone;

pub fn fmt(name: &str, ts: Timestamp) -> impl Display {
    let zoned = ts.to_zoned(TimeZone::UTC);
    fmt::from_fn(move |f| write!(f, "{name} on {} at {:.0} UTC", zoned.date(), zoned.time()))
}
