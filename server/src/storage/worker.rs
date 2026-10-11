use std::collections::HashMap;
use std::fmt::{Display, Write};
use std::fs;
use std::hash::Hash;
use std::path::PathBuf;
use std::sync::Arc;
use std::sync::mpsc::{Receiver, RecvTimeoutError, SyncSender};
use std::time::{Duration, Instant};

use anyhow::Result;
use arc_swap::Cache;
use jiff::Timestamp;
use jiff::tz::TimeZone;
use serde::Serialize;
use serde_json::ser::{PrettyFormatter, Serializer};

use crate::clients::Clients;
use crate::config::{Config, SharedConfig};
use crate::formats::online::FrameRate;
use crate::result_ext::ResultExt;
use crate::rooms::Rooms;
use crate::s3::S3;
use crate::storage::batch::Batch;
use crate::storage::init::Init;
use crate::storage::player::{Id as PlayerId, Player};
use crate::storage::race::Race;
use crate::storage::replay;
use crate::storage::stats::Stats;
use crate::util;
use crate::website::{Batch as WebsiteBatch, Message};

#[derive(Debug)]
pub struct Worker {
    config: SharedConfig,
    batch_receiver: Receiver<Batch>,
    path: PathBuf,
    tmp_path: PathBuf,
    player_numbers: HashMap<PlayerId, u64>,
    player_number: u64,
    room_numbers: HashMap<u128, u64>,
    room_number: u64,
    race_number: u64,
    buf: Vec<u8>,
    file_name_buf: String,
    tmp_path_buf: PathBuf,
    path_buf: String,
    s3: S3,
    website_message_sender: SyncSender<Message>,
    webhook_race_sender: SyncSender<Race>,
    clients: Arc<Clients>,
    rooms: Arc<Rooms>,
}

impl Worker {
    pub fn new(
        config: SharedConfig,
        batch_receiver: Receiver<Batch>,
        init: Init,
        website_message_sender: SyncSender<Message>,
        webhook_race_sender: SyncSender<Race>,
        clients: Arc<Clients>,
        rooms: Arc<Rooms>,
    ) -> Self {
        Self {
            config,
            batch_receiver,
            path: init.path,
            tmp_path: init.tmp_path,
            player_numbers: init.player_numbers,
            player_number: init.player_number,
            room_numbers: HashMap::new(),
            room_number: init.room_number,
            race_number: init.race_number,
            buf: vec![],
            file_name_buf: String::new(),
            tmp_path_buf: PathBuf::new(),
            path_buf: String::new(),
            s3: S3::default(),
            website_message_sender,
            webhook_race_sender,
            clients,
            rooms,
        }
    }

    pub fn run(mut self) -> ! {
        let mut config = Cache::new(self.config.clone());
        let mut next_tick = Instant::now();
        loop {
            let now = Instant::now();
            if let Some(duration) = next_tick.checked_duration_since(now)
                && !duration.is_zero()
            {
                let mut batch = match self.batch_receiver.recv_timeout(duration) {
                    Err(RecvTimeoutError::Timeout) => continue,
                    batch => batch.unwrap(),
                };

                let config = config.load();

                for player in &mut batch.players {
                    self.write_player(config, player).log_err();
                }
                self.write_race(config, &mut batch.race).log_err();

                let players = batch.players.clone();
                let race = batch.race.clone();

                self.write_replay(config, batch).log_err();

                let website_batch = WebsiteBatch { players, race: race.clone() };
                self.website_message_sender.try_send(Message::Batch(website_batch)).log_err();
                self.webhook_race_sender.try_send(race).log_err();

                continue;
            }

            let config = config.load();

            let room_count = FrameRate::VARIANTS
                .into_iter()
                .map(|frame_rate| self.rooms.count(frame_rate) as u64)
                .sum();
            let stats = Stats {
                dt: Timestamp::now().to_zoned(TimeZone::UTC).into(),
                player_count: self.clients.player_count() as u64,
                room_count,
            };

            self.write_stats(config, &stats).log_err();

            self.website_message_sender.try_send(Message::Stats(stats)).log_err();

            next_tick += Duration::from_secs(60);
        }
    }

    fn write_player(&mut self, config: &Config, player: &mut Player) -> Result<()> {
        player.number = self.player_number(player.id());
        self.write_json(config, player, player.number, "players")
    }

    fn write_race(&mut self, config: &Config, race: &mut Race) -> Result<()> {
        race.room_number = self.room_number(race.room_id);
        for kart in &mut race.karts {
            for player in &mut kart.players {
                let player_id = PlayerId { client_pk: kart.client_pk, profile: player.profile };
                player.number = self.player_number(player_id);
            }
        }
        race.number = self.race_number;
        self.race_number = race.number.strict_add(1);
        self.write_json(config, race, race.number, "matches")
    }

    fn write_stats(&mut self, config: &Config, stats: &Stats) -> Result<()> {
        self.write_json(config, stats, stats.dt.strftime("%Y-%m-%dT%H:%M"), "stats")
    }

    fn write_json<T: Serialize>(
        &mut self,
        config: &Config,
        x: &T,
        stem: impl Display,
        dir: &str,
    ) -> Result<()> {
        self.write(
            config,
            stem,
            "json",
            |buf| {
                let formatter = PrettyFormatter::with_indent(b"    ");
                let mut serializer = Serializer::with_formatter(buf, formatter);
                Ok(x.serialize(&mut serializer)?)
            },
            dir,
            "application/json",
        )
    }

    fn player_number(&mut self, player_id: PlayerId) -> u64 {
        number(&mut self.player_numbers, &mut self.player_number, player_id)
    }

    fn room_number(&mut self, room_id: u128) -> u64 {
        number(&mut self.room_numbers, &mut self.room_number, room_id)
    }

    fn write_replay(&mut self, config: &Config, batch: Batch) -> Result<()> {
        self.write(
            config,
            batch.race.number,
            "gkr",
            |buf| replay::write(batch, buf),
            "replays",
            "application/octet-stream",
        )
    }

    fn write(
        &mut self,
        config: &Config,
        stem: impl Display,
        ext: &str,
        write: impl FnOnce(&mut Vec<u8>) -> Result<()>,
        dir: &str,
        content_type: &str,
    ) -> Result<()> {
        self.buf.clear();
        write(&mut self.buf)?;

        self.file_name_buf.clear();
        write!(self.file_name_buf, "{stem}.{ext}")?;

        self.tmp_path.clone_into(&mut self.tmp_path_buf);
        self.tmp_path_buf.push(&self.file_name_buf);

        fs::write(&self.tmp_path_buf, &self.buf)?;

        self.path_buf.clear();
        write!(self.path_buf, "{}/{dir}/{}", self.path.display(), self.file_name_buf)?;
        fs::rename(&self.tmp_path_buf, &self.path_buf)?;

        if let Some(config) = config.s3() {
            self.path_buf.clear();
            write!(self.path_buf, "/storage/{dir}/{}", self.file_name_buf)?;

            util::retry(5, Duration::from_secs(1), || {
                self.s3.put_object(
                    config.host,
                    config.region,
                    config.access_key,
                    config.secret_key,
                    365 * 24 * 60 * 60,
                    content_type,
                    &self.path_buf,
                    &self.buf,
                )
            })?;
        }

        Ok(())
    }
}

fn number<T: Eq + Hash>(numbers: &mut HashMap<T, u64>, number: &mut u64, id: T) -> u64 {
    *numbers.entry(id).or_insert_with(|| {
        *number = number.strict_add(1);
        *number - 1
    })
}
