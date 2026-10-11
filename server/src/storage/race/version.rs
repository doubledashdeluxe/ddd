use std::fmt::{Display, Formatter, Result};

use heapless::String;
use serde::{Deserialize, Serialize};

use crate::formats::online::MAX_VERSION_LENGTH;
use crate::formats::version;

#[derive(Clone, Debug, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, Deserialize)]
pub struct Version(pub String<MAX_VERSION_LENGTH>);

impl Version {
    pub fn current() -> Self {
        Self(version::VERSION.try_into().unwrap())
    }
}

impl Default for Version {
    fn default() -> Self {
        Self("Unknown".try_into().unwrap())
    }
}

impl Display for Version {
    fn fmt(&self, f: &mut Formatter) -> Result {
        write!(f, "{}", self.0)
    }
}
