use crate::storage::Race;
use crate::website::backend::Backend;
use crate::website::html::Children;
use crate::website::last_race;
use crate::website::pack_link;
use crate::website::path_printer::PathPrinter;

pub fn write(race: &Race, backend: &mut impl Backend) {
    let write = |body: &mut Children<_>, path_printer: &PathPrinter| {
        let mut ul = body.element("ul")?.children()?;
        pack_link::write(&race.pack_hash, &mut ul, path_printer)?;
        ul.element("li")?.content(race.frame_rate)?;
        last_race::write(race, &mut ul, path_printer)?;
        ul.finish()
    };

    let name = if race.host_pk.is_some() { "Personal" } else { "Worldwide" };
    backend.write_page(
        format_args!("{name} Room #{}", race.room_number),
        write,
        "rooms",
        race.room_number,
        5,
    );
}
