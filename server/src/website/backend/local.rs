use std::fmt::{self, Display, Write};
use std::fs;

use anyhow::Result;

use crate::result_ext::ResultExt;
use crate::website::backend::Backend;
use crate::website::html::Children;
use crate::website::page;
use crate::website::path_printer::PathPrinter;

#[derive(Debug)]
pub struct Local<'a> {
    pub path: &'a str,
    pub path_buf: &'a mut String,
    pub content: &'a mut String,
}

impl Backend for Local<'_> {
    fn write_file<'a>(
        &mut self,
        content: impl Fn(&mut String) -> &[u8],
        dir_path: impl Into<Option<&'a str>>,
        file_name: impl Display,
        _: &str,
        _: u64,
    ) {
        || -> Result<_> {
            self.path.clone_into(self.path_buf);
            if let Some(dir_path) = dir_path.into() {
                write!(self.path_buf, "/{dir_path}")?;
            }
            fs::create_dir_all(&self.path_buf)?;

            write!(self.path_buf, "/{file_name}")?;
            fs::write(&self.path_buf, content(self.content))?;

            Ok(())
        }()
        .log_err();
    }

    fn write_page<'a>(
        &mut self,
        name: impl Display,
        write: impl FnOnce(&mut Children<String>, &PathPrinter) -> fmt::Result,
        dir_path: impl Into<Option<&'a str>>,
        file_name: impl Display,
        max_age: u64,
    ) {
        || -> Result<_> {
            let dir_path = dir_path.into();
            let path_printer = PathPrinter::new(dir_path, ".html");
            page::write(name, write, self.content, false, &path_printer)?;
            self.write_file(
                |content| content.as_bytes(),
                dir_path,
                format_args!("{file_name}.html"),
                "text/html",
                max_age,
            );
            Ok(())
        }()
        .log_err();
    }
}
