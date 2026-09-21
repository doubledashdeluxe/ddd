use jiff::tz::TimeZone;

use crate::storage::{Race, Stats as StorageStats};
use crate::website::backend::Backend;
use crate::website::html::Children;
use crate::website::link;
use crate::website::path_printer::PathPrinter;
use crate::website::stat::Stat;

#[derive(Debug, Default)]
pub struct Stats {
    races: Stat,
    players: Stat,
    rooms: Stat,
}

impl Stats {
    pub fn add(&mut self, race: &Race) {
        let dt = race.start.to_zoned(TimeZone::UTC).into();
        self.races.add(dt, 1);
    }

    pub fn max(&mut self, stats: &StorageStats) {
        self.players.max(stats.dt, stats.player_count);
        self.rooms.max(stats.dt, stats.room_count);
    }

    pub fn write(&self, backend: &mut impl Backend) {
        write_stats(backend);
        self.write_races(backend);
        self.write_players(backend);
        self.write_rooms(backend);
    }

    fn write_races(&self, backend: &mut impl Backend) {
        self.races.write("Matches", "matches", backend);
    }

    fn write_players(&self, backend: &mut impl Backend) {
        self.players.write("Max Players", "players", backend);
    }

    fn write_rooms(&self, backend: &mut impl Backend) {
        self.rooms.write("Max Rooms", "rooms", backend);
    }
}

fn write_stats(backend: &mut impl Backend) {
    let write = |body: &mut Children<_>, path_printer: &PathPrinter| {
        link::write("h2", "stats/matches", "Matches", body, path_printer)?;
        link::write("h2", "stats/players", "Max Players", body, path_printer)?;
        link::write("h2", "stats/rooms", "Max Rooms", body, path_printer)?;
        Ok(())
    };

    backend.write_page("Stats", write, None, "stats", 5 * 60);
}
