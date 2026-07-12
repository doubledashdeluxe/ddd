use std::fs;

use anyhow::Result;
use bpaf::{OptionParser, Parser};

fn main() -> Result<()> {
    let options = options().run();
    let mut key = [0; 32];
    getrandom::fill(&mut key)?;
    fs::write(options.key, key)?;
    Ok(())
}

fn options() -> OptionParser<Options> {
    let key = bpaf::positional("key");
    bpaf::construct!(Options { key }).to_options()
}

struct Options {
    key: String,
}
