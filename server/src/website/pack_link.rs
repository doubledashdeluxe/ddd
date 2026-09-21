use std::fmt::{Result, Write};

use crate::website::html::Children;
use crate::website::pack_name;
use crate::website::path_printer::PathPrinter;

pub fn write(
    pack_hash: &[u8; 32],
    ul: &mut Children<impl Write>,
    path_printer: &PathPrinter,
) -> Result {
    let mut li = ul.element("li")?.children()?;
    li.content("Pack:")?;
    let mut a = li.element("a")?;
    a.attribute("href")?.value(path_printer.print_page("rankings/packs"))?;
    a.content(pack_name::fmt(pack_hash))?;
    li.finish()
}
