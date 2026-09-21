use std::fmt::{Display, Result, Write};

use crate::website::html::Children;
use crate::website::path_printer::PathPrinter;

pub fn write(
    tag: &'static str,
    href: impl Display,
    content: impl Display,
    parent: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let mut element = parent.element(tag)?.children()?;
    let mut a = element.element("a")?;
    a.attribute("href")?.value(path_printer.print_page(href))?;
    a.content(content)?;
    element.finish()
}
