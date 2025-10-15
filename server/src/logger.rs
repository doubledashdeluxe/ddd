use std::fmt::Display;

use anyhow::Result;
use colored::{ColoredString, Colorize};
use log::{Level, LevelFilter, Log, Metadata, Record};

pub fn init() -> Result<()> {
    log::set_boxed_logger(Box::new(Logger))?;
    log::set_max_level(LevelFilter::Trace);
    Ok(())
}

struct Logger;

impl Log for Logger {
    fn enabled(&self, _: &Metadata) -> bool {
        true
    }

    fn log(&self, record: &Record) {
        let file = record.file().unwrap_or("?");
        let line = record.line();
        let args: ColoredString = record.args().to_string().into();
        let args = match record.level() {
            Level::Error => args.red(),
            Level::Warn => args.yellow(),
            Level::Info => args.bold(),
            Level::Debug => args,
            Level::Trace => args.dimmed(),
        };
        match line {
            Some(line) => log(file, line, args),
            None => log(file, '?', args),
        }
    }

    fn flush(&self) {}
}

fn log(file: impl Display, line: impl Display, args: impl Display) {
    eprintln!("[{file}:{line}] {args}");
}
