use std::fmt::{self, Debug, Formatter};
use std::fs;
use std::io::ErrorKind;
use std::path::Path;

use anyhow::Result;
use log::info;

pub struct Credential(pub String);

impl Credential {
    pub fn load(path: &Path, name: &str) -> Result<Option<Self>> {
        let path = path.join(name);
        let mut credential = match fs::read_to_string(&path) {
            Err(e) if e.kind() == ErrorKind::NotFound => {
                info!("{} not found", path.display());
                return Ok(None);
            }
            r => r?,
        };
        credential.truncate(credential.trim_end().len());
        Ok(Some(Self(credential)))
    }
}

impl Debug for Credential {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        f.debug_tuple("Credential").finish_non_exhaustive()
    }
}
