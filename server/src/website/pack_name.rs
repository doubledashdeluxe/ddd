use std::fmt::{self, Display};

use crate::base64;

pub fn fmt(pack_hash: &[u8; 32]) -> impl Display {
    fmt::from_fn(|f| {
        let pack = base64::display(pack_hash);
        write!(f, "{pack:.12}...")
    })
}
