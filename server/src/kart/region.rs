use std::fmt::{Display, Formatter, Result};

use serde::{Deserialize, Serialize};

#[derive(
    Clone, Copy, Debug, Default, Eq, Hash, Ord, PartialEq, PartialOrd, Serialize, Deserialize,
)]
pub enum Region {
    #[serde(rename = "P")]
    Pal,
    #[serde(rename = "U")]
    NtscU,
    #[serde(rename = "J")]
    NtscJ,
    #[default]
    #[serde(rename = "?")]
    Unknown,
}

impl Region {
    pub const fn is_unknown(&self) -> bool {
        matches!(self, Self::Unknown)
    }
}

impl Display for Region {
    fn fmt(&self, f: &mut Formatter) -> Result {
        let region = match self {
            Self::Pal => "PAL",
            Self::NtscU => "NTSC-U",
            Self::NtscJ => "NTSC-J",
            Self::Unknown => "Unknown",
        };
        write!(f, "{region}")
    }
}

impl From<u8> for Region {
    fn from(region: u8) -> Self {
        match region.into() {
            'P' => Self::Pal,
            'E' => Self::NtscU,
            'J' => Self::NtscJ,
            _ => Self::Unknown,
        }
    }
}
