use crate::formats::online::ModeIndex;
use crate::storage::{Player, Race};
use crate::website::backend::Backend;
use crate::website::duration::Duration;
use crate::website::html::Children;
use crate::website::last_race;
use crate::website::path_printer::PathPrinter;

pub fn write(race: &Race, player: &Player, backend: &mut impl Backend) {
    let write = |body: &mut Children<_>, path_printer: &PathPrinter| {
        let mut ul = body.element("ul")?.children()?;

        ul.element("li")?.content(format_args!("Index: {}", player.index))?;

        for mode_index in ModeIndex::VARIANTS {
            let Some(mmr) = player.mmrs.get(&mode_index) else { continue };
            ul.element("li")?.content(format_args!("{mode_index} MMR: {mmr}"))?;
        }

        ul.element("li")?.content(format_args!("Matches: {}", player.race_count))?;

        let play_time = Duration(player.play_time);
        ul.element("li")?.content(format_args!("Play time: {play_time}"))?;

        last_race::write(race, &mut ul, path_printer)?;

        ul.finish()
    };

    backend.write_page(
        format_args!("{} · Player #{}", player.name, player.number),
        write,
        "players",
        player.number,
        5,
    );
}
