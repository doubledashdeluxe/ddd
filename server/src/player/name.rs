use std::fmt::{self, Display, Formatter};

use anyhow::{Error, Result};
use serde::de::Error as _;
use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Clone, Copy, Debug)]
pub struct Name([u8; 3]);

impl Display for Name {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        let name = str::from_utf8(&self.0).unwrap_or("   ");
        write!(f, "{name}")
    }
}

impl TryFrom<[u8; 3]> for Name {
    type Error = Error;

    fn try_from(name: [u8; 3]) -> Result<Self> {
        for c in name {
            match c {
                b'A'..=b'Z' => (),
                b'0'..=b'9' => (),
                b'+' => (),
                b'-' => (),
                b'!' => (),
                b'?' => (),
                b'.' => (),
                b' ' => (),
                _ => anyhow::bail!("invalid character"),
            }
        }
        Ok(Self(name))
    }
}

impl From<Name> for [u8; 3] {
    fn from(name: Name) -> Self {
        name.0
    }
}

impl Serialize for Name {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        str::from_utf8(&self.0).unwrap_or("   ").serialize(serializer)
    }
}

impl<'de> Deserialize<'de> for Name {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        let name = <&str>::deserialize(deserializer)?;
        Self::try_from(name.as_bytes().try_into().unwrap_or(*b"   ")).map_err(D::Error::custom)
    }
}
