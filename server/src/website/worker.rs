use std::collections::HashMap;
use std::sync::Arc;
use std::sync::mpsc::Receiver;

use arc_swap::Cache;
use jiff::Timestamp;
use jiff::tz::TimeZone;

use crate::config::SharedConfig;
use crate::courses::{Courses, SharedCourses};
use crate::rooms::Rooms;
use crate::s3::S3;
use crate::storage::{Batch, Stats as StorageStats};
use crate::website::backend::{Backend, Local as LocalBackend, S3 as S3Backend};
use crate::website::css;
use crate::website::init::Init;
use crate::website::message::Message;
use crate::website::player;
use crate::website::race;
use crate::website::room;
use crate::website::rooms;
use crate::website::state::State;

pub struct Worker {
    config: SharedConfig,
    courses: SharedCourses,
    message_receiver: Receiver<Message>,
    state: State,
    path: String,
    content: String,
    path_buf: String,
    s3: S3,
    rooms: Arc<Rooms>,
}

impl Worker {
    pub fn new(
        config: SharedConfig,
        courses: SharedCourses,
        message_receiver: Receiver<Message>,
        init: Init,
        path: &str,
        rooms: Arc<Rooms>,
    ) -> Self {
        let state = State {
            player_numbers: init.player_numbers,
            player_names: init.player_names,
            room_numbers: HashMap::new(),
            rankings: init.rankings,
            stats: init.stats,
            counter: 0,
        };

        Self {
            config,
            courses,
            message_receiver,
            state,
            path: format!("{path}/website"),
            content: String::new(),
            path_buf: String::new(),
            s3: S3::default(),
            rooms,
        }
    }

    pub fn run(mut self) -> ! {
        let mut config = Cache::new(self.config.clone());
        let mut courses = Cache::new(self.courses.clone());
        loop {
            let mut message = self.message_receiver.recv().unwrap();

            let config = config.load();
            let courses = courses.load();

            if let (Some(host), Some(region), Some(access_key), Some(secret_key)) =
                (&config.s3_host, &config.s3_region, &config.s3_access_key, &config.s3_secret_key)
            {
                let mut backend = S3Backend {
                    path_buf: &mut self.path_buf,
                    content: &mut self.content,
                    s3: &mut self.s3,
                    host,
                    region,
                    access_key,
                    secret_key,
                };
                process(courses, &mut self.state, &self.rooms, &mut message, &mut backend);
            } else {
                let mut backend = LocalBackend {
                    path: &self.path,
                    path_buf: &mut self.path_buf,
                    content: &mut self.content,
                };
                process(courses, &mut self.state, &self.rooms, &mut message, &mut backend);
            }
        }
    }
}

fn process(
    courses: &Courses,
    state: &mut State,
    rooms: &Rooms,
    message: &mut Message,
    backend: &mut impl Backend,
) {
    match message {
        Message::Batch(batch) => process_batch(courses, state, batch, backend),
        Message::Stats(stats) => process_stats(courses, state, rooms, stats, backend),
    }
}

fn process_batch(
    courses: &Courses,
    state: &mut State,
    batch: &mut Batch,
    backend: &mut impl Backend,
) {
    race::write(courses, &mut batch.race, backend);
    room::write(&batch.race, backend);
    for player in &batch.players {
        player::write(&batch.race, player, backend);
    }

    state.room_numbers.insert(batch.race.room_id, batch.race.room_number);
    for player in &batch.players {
        state.player_numbers.insert(player.id(), player.number);
        state.player_names.insert(player.number, player.name);
    }

    let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
    state.rankings.add(now, &batch.race);
    state.stats.add(&batch.race);
}

fn process_stats(
    courses: &Courses,
    state: &mut State,
    rooms: &Rooms,
    stats: &StorageStats,
    backend: &mut impl Backend,
) {
    let now = Timestamp::now().to_zoned(TimeZone::UTC).into();
    state.rankings.update(now);
    state.stats.max(stats);

    if state.counter == 0 {
        backend.write_file(|_| css::CONTENT, "css", css::FILE_NAME, "text/css", 24 * 60 * 60);
        backend.write_file(
            |_| include_bytes!("favicon.ico"),
            None,
            "favicon.ico",
            "image/vnd.microsoft.icon",
            24 * 60 * 60,
        );
    }
    if state.counter.is_multiple_of(15) {
        state.rankings.write(courses, &state.player_names, backend);
        state.stats.write(backend);
    }
    rooms::write(&state.player_numbers, &state.room_numbers, rooms, backend);

    state.counter += 1;
}
