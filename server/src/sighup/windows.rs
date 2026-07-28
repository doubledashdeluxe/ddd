use std::iter;
use std::thread;
use std::time::Duration;

use anyhow::Result;

pub fn sighup() -> Result<impl Iterator<Item = ()>> {
    thread::sleep(Duration::MAX);
    Ok(iter::empty())
}
