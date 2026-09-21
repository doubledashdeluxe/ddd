use std::cmp::Reverse;
use std::fmt::{Display, Result, Write};

use crate::sorted::Sorted;
use crate::website::html::{Children, Element};
use crate::website::path_printer::PathPrinter;
use crate::website::rank::Rank;

pub fn write<T, C: Default + Display + PartialEq, W: Write>(
    name: impl Display,
    sorted: &Sorted<T, Reverse<C>>,
    sum: Option<&C>,
    write_value: impl Fn(&T, Element<W>, &PathPrinter) -> Result,
    parent: &mut Children<W>,
    path_printer: &PathPrinter,
) -> Result {
    let mut div = parent.element("div")?;
    div.attribute("class")?.value("ranking")?;
    let mut div = div.children()?;

    div.element("h3")?.content(name)?;

    let mut table = div.element("table")?.children()?;
    for (rank, (counter, value)) in sorted.values().iter().enumerate() {
        if rank >= 50 || counter.0 == Default::default() {
            break;
        }

        let mut tr = table.element("tr")?.children()?;

        tr.element("td")?.content(Rank(rank))?;

        let td = tr.element("td")?;
        write_value(value, td, path_printer)?;

        tr.element("td")?.content(&counter.0)?;

        tr.finish()?;
    }
    if let Some(sum) = sum {
        let mut tr = table.element("tr")?.children()?;

        tr.element("td")?.empty()?;
        tr.element("td")?.content("Total")?;
        tr.element("td")?.content(sum)?;

        tr.finish()?;
    }
    table.finish()?;

    div.finish()
}
