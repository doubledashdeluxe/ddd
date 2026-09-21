use std::fmt::{Result, Write};

use crate::storage::Race;
use crate::website::html::Children;
use crate::website::path_printer::PathPrinter;

pub fn write(race: &Race, ul: &mut Children<impl Write>, path_printer: &PathPrinter) -> Result {
    let mut li = ul.element("li")?.children()?;
    li.content("Last match:")?;
    let mut a = li.element("a")?;
    a.attribute("href")?.value(path_printer.print_page(format_args!("matches/{}", race.number)))?;
    a.content(format_args!("{} #{}", race.mode, race.number))?;
    li.finish()
}
