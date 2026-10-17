use anyhow::Result;
use signal_hook::consts::SIGHUP;
use signal_hook::iterator::Signals;

pub struct Sighup(Signals);

impl Sighup {
    pub fn new() -> Result<Self> {
        Ok(Self(Signals::new([SIGHUP])?))
    }

    pub fn wait(&mut self) -> impl Iterator<Item = ()> {
        self.0.wait().map(|_| ())
    }
}
