use std::collections::HashMap;
use std::fmt::Write;
use std::fs;
use std::hash::Hash;
use std::path::PathBuf;
use std::sync::mpsc::Receiver;

use anyhow::Result;
use log::error;
use serde::Serialize;

use crate::storage::batch::Batch;
use crate::storage::init::Init;
use crate::storage::player::{Id as PlayerId, Player};
use crate::storage::race::Race;

#[derive(Debug)]
pub struct Worker {
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
    path_buf: PathBuf,
}

impl Worker {
    pub fn new(batch_receiver: Receiver<Batch>, init: Init) -> Self {
        Self {
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
            path_buf: PathBuf::new(),
        }
    }

    pub fn run(mut self) -> ! {
        loop {
            let mut batch = self.batch_receiver.recv().unwrap();
            for player in &batch.players {
                if let Err(e) = self.write_player(player) {
                    error!("{e}");
                }
            }
            if let Err(e) = self.write_race(&mut batch.race) {
                error!("{e}");
            }
        }
    }

    fn write_player(&mut self, player: &Player) -> Result<()> {
        let player_number = self.player_number(player.id());
        self.write(player, player_number, "players")
    }

    fn write_race(&mut self, race: &mut Race) -> Result<()> {
        race.room_number = self.room_number(race.room_id);
        for kart in &mut race.karts {
            for player in &mut kart.players {
                let player_id = PlayerId { client_pk: kart.client_pk, index: player.index };
                player.number = self.player_number(player_id);
            }
        }
        let race_number = self.race_number;
        self.race_number = race_number.strict_add(1);
        self.write(race, race_number, "races")
    }

    fn write<T: Serialize>(&mut self, x: &T, number: u64, dir: &str) -> Result<()> {
        self.buf.clear();
        serde_json::to_writer_pretty(&mut self.buf, x)?;

        self.file_name_buf.clear();
        write!(self.file_name_buf, "{number}.json")?;

        self.tmp_path.clone_into(&mut self.tmp_path_buf);
        self.tmp_path_buf.push(&self.file_name_buf);

        fs::write(&self.tmp_path_buf, &self.buf)?;

        self.path.clone_into(&mut self.path_buf);
        self.path_buf.push(dir);
        self.path_buf.push(&self.file_name_buf);
        fs::rename(&self.tmp_path_buf, &self.path_buf)?;

        Ok(())
    }

    fn player_number(&mut self, player_id: PlayerId) -> u64 {
        number(&mut self.player_numbers, &mut self.player_number, player_id)
    }

    fn room_number(&mut self, room_id: u128) -> u64 {
        number(&mut self.room_numbers, &mut self.room_number, room_id)
    }
}

fn number<T: Eq + Hash>(numbers: &mut HashMap<T, u64>, number: &mut u64, id: T) -> u64 {
    *numbers.entry(id).or_insert_with(|| {
        *number = number.strict_add(1);
        *number - 1
    })
}
