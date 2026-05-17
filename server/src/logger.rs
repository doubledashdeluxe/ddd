use std::fmt::Display;
use std::io::{self, IsTerminal};

use anyhow::Result;
use log::{Level, LevelFilter, Log, Metadata, Record};
use yansi::{Condition, Paint};

pub fn init() -> Result<()> {
    let stderr = io::stderr();
    let condition = Condition::clicolor() && Condition::no_color() && stderr.is_terminal();
    let condition = Condition::cached(condition);
    yansi::whenever(condition);

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
        let args = record.args();
        let args = match record.level() {
            Level::Error => args.red().bold(),
            Level::Warn => args.yellow().bold(),
            Level::Info => args.bold(),
            Level::Debug => args.white(),
            Level::Trace => args.dim(),
        };
        match line {
            Some(line) => log(file, line, args),
            None => log(file, '?', args),
        }
    }

    fn flush(&self) {}
}

#[expect(clippy::print_stderr)]
fn log(file: impl Display, line: impl Display, args: impl Display) {
    eprintln!("[{file}:{line}] {args}");
}
