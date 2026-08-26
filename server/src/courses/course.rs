use std::fmt::{Display, Formatter, Result};

use serde::Deserialize;

use crate::base64;

#[derive(Debug, Deserialize)]
pub struct Course {
    #[serde(skip)]
    pub name: String,
    pub author: String,
    #[serde(with = "base64")]
    pub hash: [u8; 32],
}

impl Display for Course {
    fn fmt(&self, f: &mut Formatter) -> Result {
        write!(f, "{} ({})", self.name, self.author)
    }
}
