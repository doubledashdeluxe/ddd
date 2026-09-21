use std::fmt::Display;

use log::error;

pub trait ResultExt<T> {
    fn log_err(self) -> Option<T>;
}

impl<T, E: Display> ResultExt<T> for Result<T, E> {
    fn log_err(self) -> Option<T> {
        if let Err(e) = &self {
            error!("{e}");
        }
        self.ok()
    }
}
