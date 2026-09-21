use std::cmp::Reverse;
use std::collections::HashMap;
use std::fmt::{self, Display, Result, Write};
use std::time::SystemTime;

use jiff::civil::Date;
use jiff::tz::TimeZone;

use crate::courses::Courses;
use crate::formats::online::{CharacterId, KartId, MODE_INDEX_COUNT, ModeIndex};
use crate::kart::{Platform, Region};
use crate::player::Name;
use crate::sorted::Sorted;
use crate::storage::Race;
use crate::website::backend::Backend;
use crate::website::combo::Combo;
use crate::website::counter_ranking::CounterRanking;
use crate::website::course_name;
use crate::website::duration::Duration;
use crate::website::html::{Children, Element};
use crate::website::link;
use crate::website::pack_name;
use crate::website::path_printer::PathPrinter;
use crate::website::ranking;

#[derive(Debug, Default)]
pub struct Rankings {
    regions: CounterRanking<Region, u64>,
    platforms: CounterRanking<Platform, u64>,
    player_mmrs: [Sorted<u64, Reverse<u16>>; MODE_INDEX_COUNT],
    player_races: CounterRanking<u64, u64>,
    player_times: CounterRanking<u64, Duration>,
    courses: CounterRanking<[u8; 32], u64>,
    packs: CounterRanking<[u8; 32], u64>,
    characters: CounterRanking<CharacterId, u64>,
    karts: CounterRanking<KartId, u64>,
    combos: CounterRanking<Combo, u64>,
}

impl Rankings {
    pub fn add(&mut self, now: Date, race: &Race) {
        let date = race.start.to_zoned(TimeZone::UTC).into();
        self.courses.add(now, date, race.course_hash, 1);
        self.packs.add(now, date, race.pack_hash, 1);
        let start = SystemTime::from(race.start);
        let end = SystemTime::from(race.end);
        let duration = end.duration_since(start).unwrap_or_default();
        let duration = Duration(duration);
        for kart in &race.karts {
            self.regions.add(now, date, kart.region, 1);
            self.platforms.add(now, date, kart.platform.clone(), 1);
            for player in &kart.players {
                self.player_races.add(now, date, player.number, 1);
                self.player_times.add(now, date, player.number, duration);
            }
            for character in kart.characters {
                self.characters.add(now, date, character, 1);
            }
            self.karts.add(now, date, kart.kart, 1);
            let combo = Combo::new(kart.characters, kart.kart);
            self.combos.add(now, date, combo, 1);
        }
    }

    pub fn update(&mut self, now: Date) {
        self.regions.update(now);
        self.platforms.update(now);
        self.player_races.update(now);
        self.player_times.update(now);
        self.courses.update(now);
        self.packs.update(now);
        self.characters.update(now);
        self.karts.update(now);
        self.combos.update(now);
    }

    pub fn write(
        &self,
        courses: &Courses,
        player_names: &HashMap<u64, Name>,
        backend: &mut impl Backend,
    ) {
        write_rankings(backend);
        self.write_regions(backend);
        self.write_platforms(backend);
        self.write_players(player_names, backend);
        self.write_courses(courses, backend);
        self.write_packs(backend);
        self.write_characters(backend);
        self.write_karts(backend);
        self.write_combos(backend);
    }

    fn write_regions(&self, backend: &mut impl Backend) {
        write(
            "Region",
            write_counter_ranking("Occurrences", &self.regions),
            |region, td, _| td.content(region),
            backend,
            "regions",
        );
    }

    fn write_platforms(&self, backend: &mut impl Backend) {
        write(
            "Platform",
            write_counter_ranking("Occurrences", &self.platforms),
            |platform, td, _| td.content(platform),
            backend,
            "platforms",
        );
    }

    fn write_players(&self, player_names: &HashMap<u64, Name>, backend: &mut impl Backend) {
        write(
            "Player",
            |wv, b, pp| {
                write_ranking("MMR", |d, pp| self.write_player_mmrs(wv, d, pp), b, pp)?;
                write_counter_ranking("Matches", &self.player_races)(wv, b, pp)?;
                write_counter_ranking("Play time", &self.player_times)(wv, b, pp)?;
                Ok(())
            },
            |player, td, path_printer| {
                let mut td = td.children()?;
                let mut a = td.element("a")?;
                let href = format_args!("players/{player}");
                a.attribute("href")?.value(path_printer.print_page(href))?;
                let player = fmt::from_fn(|f| {
                    if let Some(name) = player_names.get(player) {
                        write!(f, "{name}")
                    } else {
                        write!(f, "   ")
                    }
                });
                a.content(player)?;
                td.finish()
            },
            backend,
            "players",
        );
    }

    fn write_player_mmrs<W: Write>(
        &self,
        write_value: impl Fn(&u64, Element<W>, &PathPrinter) -> Result,
        div: &mut Children<W>,
        path_printer: &PathPrinter,
    ) -> Result {
        for mode_index in ModeIndex::VARIANTS {
            ranking::write(
                mode_index,
                &self.player_mmrs[mode_index as usize],
                None,
                &write_value,
                div,
                path_printer,
            )?;
        }
        Ok(())
    }

    fn write_courses(&self, courses: &Courses, backend: &mut impl Backend) {
        write(
            "Course",
            write_counter_ranking("Matches", &self.courses),
            |course, td, _| td.content(course_name::fmt(courses, course)),
            backend,
            "courses",
        );
    }

    fn write_packs(&self, backend: &mut impl Backend) {
        write(
            "Pack",
            write_counter_ranking("Matches", &self.packs),
            |pack, td, _| td.content(pack_name::fmt(pack)),
            backend,
            "packs",
        );
    }

    fn write_characters(&self, backend: &mut impl Backend) {
        write(
            "Character",
            write_counter_ranking("Picks", &self.characters),
            |character, td, _| td.content(character),
            backend,
            "characters",
        );
    }

    fn write_karts(&self, backend: &mut impl Backend) {
        write(
            "Kart",
            write_counter_ranking("Picks", &self.karts),
            |kart, td, _| td.content(kart),
            backend,
            "karts",
        );
    }

    fn write_combos(&self, backend: &mut impl Backend) {
        write(
            "Combo",
            write_counter_ranking("Picks", &self.combos),
            |combo, td, _| td.content(combo),
            backend,
            "combos",
        );
    }
}

fn write_rankings(backend: &mut impl Backend) {
    let write = |body: &mut Children<_>, path_printer: &PathPrinter| {
        link::write("h2", "rankings/regions", "Regions", body, path_printer)?;
        link::write("h2", "rankings/platforms", "Platforms", body, path_printer)?;
        link::write("h2", "rankings/players", "Players", body, path_printer)?;
        link::write("h2", "rankings/courses", "Courses", body, path_printer)?;
        link::write("h2", "rankings/packs", "Packs", body, path_printer)?;
        link::write("h2", "rankings/characters", "Characters", body, path_printer)?;
        link::write("h2", "rankings/karts", "Karts", body, path_printer)?;
        link::write("h2", "rankings/combos", "Combos", body, path_printer)?;
        Ok(())
    };

    backend.write_page("Rankings", write, None, "rankings", 5 * 60);
}

fn write<T, V: Fn(&T, Element<String>, &PathPrinter) -> Result>(
    name: &str,
    write: impl FnOnce(V, &mut Children<String>, &PathPrinter) -> Result,
    write_value: V,
    backend: &mut impl Backend,
    file_name: impl Display,
) {
    backend.write_page(
        format_args!("{name} Rankings"),
        |body, path_printer| write(write_value, body, path_printer),
        "rankings",
        file_name,
        5 * 60,
    );
}

fn write_counter_ranking<
    T,
    C: Default + Display + PartialEq,
    V: Fn(&T, Element<W>, &PathPrinter) -> Result,
    W: Write,
>(
    counter_name: &str,
    ranking: &CounterRanking<T, C>,
) -> impl FnOnce(V, &mut Children<W>, &PathPrinter) -> Result {
    move |write_value, body, path_printer| {
        write_ranking(
            counter_name,
            |div, path_printer| {
                ranking.write(
                    |value, td, path_printer| write_value(value, td, path_printer),
                    div,
                    path_printer,
                )
            },
            body,
            path_printer,
        )
    }
}

fn write_ranking<W: Write>(
    counter_name: &str,
    write_ranking: impl FnOnce(&mut Children<W>, &PathPrinter) -> Result,
    body: &mut Children<W>,
    path_printer: &PathPrinter,
) -> Result {
    body.element("h2")?.content(counter_name)?;
    let mut div = body.element("div")?;
    div.attribute("class")?.value("rankings")?;
    let mut div = div.children()?;
    write_ranking(&mut div, path_printer)?;
    div.finish()
}
