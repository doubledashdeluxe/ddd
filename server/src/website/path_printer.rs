use std::fmt::{self, Display};

pub struct PathPrinter<'a> {
    prefix: &'a str,
    page_suffix: &'a str,
}

impl<'a> PathPrinter<'a> {
    pub const fn new(dir_path: Option<&'a str>, page_suffix: &'a str) -> Self {
        let prefix = if dir_path.is_some() { "../" } else { "" };
        Self { prefix, page_suffix }
    }

    pub fn print_file(&self, path: impl Display) -> impl Display {
        fmt::from_fn(move |f| write!(f, "{}{path}", self.prefix))
    }

    pub fn print_page(&self, path: impl Display) -> impl Display {
        fmt::from_fn(move |f| write!(f, "{}{path}{}", self.prefix, self.page_suffix))
    }
}
