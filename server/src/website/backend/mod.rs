pub use crate::website::backend::local::Local;
pub use crate::website::backend::s3::S3;

use std::fmt::{self, Display};

use crate::website::html::Children;
use crate::website::path_printer::PathPrinter;

mod local;
mod s3;

pub trait Backend {
    fn write_file<'a>(
        &mut self,
        content: impl Fn(&mut String) -> &[u8],
        dir_path: impl Into<Option<&'a str>>,
        file_name: impl Display,
        content_type: &str,
        max_age: u64,
    );

    fn write_page<'a>(
        &mut self,
        name: impl Display,
        write: impl FnOnce(&mut Children<String>, &PathPrinter) -> fmt::Result,
        dir_path: impl Into<Option<&'a str>>,
        file_name: impl Display,
        max_age: u64,
    );
}
