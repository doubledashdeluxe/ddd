use std::env;
use std::fmt;
use std::io::{self, IsTerminal};

use anyhow::Result;
use jiff::Zoned;
use log::{Level, LevelFilter, Log, Metadata, Record};
use yansi::{Condition, Paint};

pub fn init(max_level: LevelFilter) -> Result<()> {
    let stderr = io::stderr();
    let condition = Condition::clicolor() && Condition::no_color() && stderr.is_terminal();
    let condition = Condition::cached(condition);
    yansi::whenever(condition);

    log::set_boxed_logger(Box::new(Logger))?;
    log::set_max_level(max_level);
    Ok(())
}

struct Logger;

impl Log for Logger {
    fn enabled(&self, _: &Metadata) -> bool {
        true
    }

    #[expect(clippy::print_stderr)]
    fn log(&self, record: &Record) {
        let file = record.file().and_then(|file| Some(file.rsplit_once('/')?.1)).unwrap_or("?");
        let line = fmt::from_fn(|f| match record.line() {
            Some(line) => write!(f, "{line}"),
            None => write!(f, "?"),
        });
        let args = record.args();

        if env::var_os("JOURNAL_STREAM").is_some() {
            let level = match record.level() {
                Level::Error => "<3>",
                Level::Warn => "<4>",
                Level::Info => "<6>",
                Level::Debug => "<7>",
                Level::Trace => "<7>",
            };
            eprintln!("{level}[{file}:{line}] {args}");
            return;
        }

        let time = Zoned::now().time();
        let args = match record.level() {
            Level::Error => args.red().bold(),
            Level::Warn => args.yellow().bold(),
            Level::Info => args.bright_white(),
            Level::Debug => args.dim(),
            Level::Trace => args.bright_black(),
        };
        eprintln!("{} {args}", format_args!("[{time:.3}] [{file}:{line}]").bright_white());
    }

    fn flush(&self) {}
}
