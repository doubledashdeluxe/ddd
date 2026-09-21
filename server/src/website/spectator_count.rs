use std::fmt::{Result, Write};

use crate::website::html::Children;

pub fn write(spectator_count: u64, ul: &mut Children<impl Write>) -> Result {
    let name = if spectator_count == 1 { "Spectator" } else { "Spectators" };
    ul.element("li")?.content(format_args!("{spectator_count} {name}"))
}
