use std::fmt::{self, Display, Write};

use anyhow::Result;

use crate::credential::Credential;
use crate::result_ext::ResultExt;
use crate::s3;
use crate::website::backend::Backend;
use crate::website::html::Children;
use crate::website::page;
use crate::website::path_printer::PathPrinter;

#[derive(Debug)]
pub struct S3<'a> {
    pub path_buf: &'a mut String,
    pub content: &'a mut String,
    pub s3: &'a mut s3::S3,
    pub host: &'a str,
    pub region: &'a str,
    pub access_key: &'a Credential,
    pub secret_key: &'a Credential,
}

impl Backend for S3<'_> {
    fn write_file<'a>(
        &mut self,
        content: impl Fn(&mut String) -> &[u8],
        dir_path: impl Into<Option<&'a str>>,
        file_name: impl Display,
        content_type: &str,
        max_age: u64,
    ) {
        || -> Result<_> {
            self.path_buf.clear();
            if let Some(dir_path) = dir_path.into() {
                write!(self.path_buf, "/{dir_path}/{file_name}")?;
            } else {
                write!(self.path_buf, "/{file_name}")?;
            }
            self.s3.put_object(
                self.host,
                self.region,
                self.access_key,
                self.secret_key,
                max_age,
                content_type,
                self.path_buf,
                content(self.content),
            )
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
            let path_printer = PathPrinter::new(dir_path, "");
            page::write(name, write, self.content, true, &path_printer)?;
            self.write_file(
                |content| content.as_bytes(),
                dir_path,
                file_name,
                "text/html",
                max_age,
            );
            Ok(())
        }()
        .log_err();
    }
}
