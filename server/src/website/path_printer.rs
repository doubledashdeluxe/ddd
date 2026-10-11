use std::fmt::{self, Display};

pub struct PathPrinter<'a> {
    prefix: &'a str,
    page_suffix: &'a str,
    storage_prefix: &'a str,
}

impl<'a> PathPrinter<'a> {
    pub const fn new(
        dir_path: Option<&'a str>,
        page_suffix: &'a str,
        storage_prefix: &'a str,
    ) -> Self {
        let prefix = if dir_path.is_some() { "../" } else { "" };
        Self { prefix, page_suffix, storage_prefix }
    }

    pub fn print_file(&self, path: impl Display) -> impl Display {
        fmt::from_fn(move |f| write!(f, "{}{path}", self.prefix))
    }

    pub fn print_page(&self, path: impl Display) -> impl Display {
        fmt::from_fn(move |f| write!(f, "{}{path}{}", self.prefix, self.page_suffix))
    }

    pub fn print_storage_file(&self, path: impl Display) -> impl Display {
        fmt::from_fn(move |f| write!(f, "{}{}{path}", self.prefix, self.storage_prefix))
    }
}
