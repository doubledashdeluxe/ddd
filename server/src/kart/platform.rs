use std::fmt::{Display, Formatter, Result};

use heapless::{String, Vec};
use serde::{Deserialize, Serialize};

use crate::formats::online::MAX_PLATFORM_LENGTH;

#[derive(Clone, Debug, Default, Eq, Hash, PartialEq, PartialOrd, Ord, Serialize, Deserialize)]
#[serde(transparent)]
pub struct Platform(String<MAX_PLATFORM_LENGTH>);

impl Platform {
    pub fn is_empty(&self) -> bool {
        self.0.is_empty()
    }
}

impl Display for Platform {
    fn fmt(&self, f: &mut Formatter) -> Result {
        let platform = if self.0.is_empty() { "Unknown" } else { &self.0 };
        write!(f, "{platform}")
    }
}

impl From<Vec<u8, MAX_PLATFORM_LENGTH>> for Platform {
    fn from(platform: Vec<u8, MAX_PLATFORM_LENGTH>) -> Self {
        let platform = String::from_utf8(platform)
            .ok()
            .filter(|platform| platform.chars().all(|c| matches!(c, ' '..='~')))
            .unwrap_or_default();
        Self(platform)
    }
}
