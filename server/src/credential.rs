use std::fmt::{Debug, Formatter, Result};
use std::fs;

pub struct Credential(pub String);

impl Credential {
    pub fn load(path: &str) -> Option<Self> {
        let mut credential = fs::read_to_string(path).ok()?;
        credential.truncate(credential.trim_end().len());
        Some(Self(credential))
    }
}

impl Debug for Credential {
    fn fmt(&self, f: &mut Formatter) -> Result {
        f.debug_tuple("Credential").finish_non_exhaustive()
    }
}
