use std::iter;
use std::thread;
use std::time::Duration;

use anyhow::Result;

#[expect(clippy::unnecessary_wraps)]
pub fn sighup() -> Result<impl Iterator<Item = ()>> {
    thread::sleep(Duration::MAX);
    Ok(iter::empty())
}
