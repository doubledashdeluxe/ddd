use std::iter;
use std::thread;
use std::time::Duration;

use anyhow::Result;

pub struct Sighup;

impl Sighup {
    pub const fn new() -> Result<Self> {
        Ok(Self)
    }

    pub fn wait(&mut self) -> impl Iterator<Item = ()> {
        thread::sleep(Duration::MAX);
        iter::empty()
    }
}
