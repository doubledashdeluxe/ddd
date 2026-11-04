use std::cmp::Ordering;
use std::fmt::{self, Debug, Formatter};
use std::fs;
use std::io::ErrorKind;
use std::path::Path;
use std::str::FromStr;

use anyhow::{Error, Result};
use log::info;

#[derive(Default)]
pub struct DeniedNames(Vec<heapless::Vec<u8, 6>>);

impl DeniedNames {
    pub fn read(path: &Path) -> Result<Self> {
        let path = path.join("denied-names");
        match fs::read_to_string(&path) {
            Err(e) if e.kind() == ErrorKind::NotFound => {
                info!("{} not found, all names are allowed", path.display());
                Ok(Self::default())
            }
            r => Ok(r?.parse()?),
        }
    }

    pub fn contains(&self, names: &mut [[u8; 3]]) -> bool {
        let names = names.as_flattened_mut();
        for c in &mut *names {
            *c = match *c {
                b'!' => b'I',
                b'0' => b'O',
                b'1' => b'I',
                b'3' => b'E',
                b'4' => b'A',
                b'5' => b'S',
                b'6' => b'G',
                b'7' => b'T',
                b'8' => b'B',
                c => c,
            }
        }
        (0..names.len()).take_while(|offset| offset + 3 <= names.len()).any(|offset| {
            let names = &names[offset..];
            self.0
                .binary_search_by(|name| {
                    if names.starts_with(name) {
                        Ordering::Equal
                    } else {
                        name.as_slice().cmp(names)
                    }
                })
                .is_ok()
        })
    }
}

impl Debug for DeniedNames {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "DeniedNames({})", self.0.len())
    }
}

impl FromStr for DeniedNames {
    type Err = Error;

    fn from_str(names: &str) -> Result<Self, Self::Err> {
        let names: Result<Vec<heapless::Vec<_, _>>> =
            names.lines().map(|name| Ok(name.as_bytes().try_into()?)).collect();
        let mut names = names?;
        for name in &mut names {
            name.make_ascii_uppercase();
        }
        names.sort();
        Ok(Self(names))
    }
}
