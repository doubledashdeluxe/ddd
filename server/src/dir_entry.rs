use std::ffi::OsStr;
use std::fs::{self, DirEntry};
use std::str::FromStr;

use anyhow::{Context, Error, Result};
use serde::de::DeserializeOwned;

pub fn extract_json_stem<T: FromStr>(entry: &DirEntry, name: &str) -> Result<T>
where
    Error: From<T::Err>,
{
    let extract_json_stem = |file_name: &OsStr| -> Result<_> {
        let file_name = file_name.to_str().context("invalid UTF-8")?;
        let stem = file_name.strip_suffix(".json").context("missing .json extension")?;
        Ok(stem.parse()?)
    };

    let file_name = entry.file_name();
    extract_json_stem(&file_name)
        .with_context(|| format!("invalid {name} file name {}", file_name.display()))
}

pub fn read_json<T: DeserializeOwned>(entry: &DirEntry, name: &str) -> Result<T> {
    let file_name = entry.file_name();
    let path = entry.path();
    let x = fs::read_to_string(path)
        .with_context(|| format!("could not read {name} file {}", file_name.display()))?;
    let x = serde_json::from_str(&x)
        .with_context(|| format!("invalid {name} file {}", file_name.display()))?;
    Ok(x)
}
