use std::iter;

use anyhow::Result;
use signal_hook::consts::SIGHUP;
use signal_hook::iterator::Signals;

pub fn sighup() -> Result<impl Iterator<Item = ()>> {
    let mut signals = Signals::new([SIGHUP])?;
    Ok(iter::repeat_with(move || signals.wait()).flatten().map(|_| ()))
}
