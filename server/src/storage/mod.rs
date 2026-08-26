pub use crate::storage::player::Id as PlayerId;
pub use crate::storage::player::Player;
pub use crate::storage::race::Race;
pub use crate::storage::worker::Worker;

use std::fs::{self, DirEntry};
use std::path::Path;
use std::sync::mpsc::SyncSender;

use anyhow::{Context, Result, anyhow};
use heapless::Vec;
use scc::HashMap;

use crate::dir_entry;
use crate::formats::online::*;
use crate::storage::batch::Batch;
use crate::storage::init::Init;

pub mod race;

mod batch;
mod init;
mod player;
mod worker;

#[derive(Debug)]
pub struct Storage {
    batch_sender: SyncSender<Batch>,
    players: HashMap<PlayerId, Player>,
}

impl Storage {
    pub fn load(batch_sender: SyncSender<Batch>, path: impl AsRef<Path>) -> Result<(Self, Init)> {
        let path = path.as_ref();

        let tmp_path = path.join("tmp");
        fs::create_dir_all(&tmp_path)?;

        let mut init = Init::new(path.to_owned(), tmp_path);

        let players_path = path.join("players");
        fs::create_dir_all(&players_path)?;
        let players = HashMap::new();
        for entry in fs::read_dir(&players_path)? {
            let entry = entry?;

            let number = update_number(&entry, &mut init.player_number, "player")?;

            let player: Player = dir_entry::read_json(&entry, "player")?;
            let id = player.id();
            players.insert_sync(id, player).map_err(|_| anyhow!("duplicate player {id:?}"))?;

            init.player_numbers.insert(id, number);
        }

        let races_path = path.join("races");
        fs::create_dir_all(&races_path)?;
        for entry in fs::read_dir(&races_path)? {
            let entry = entry?;

            let number = update_number(&entry, &mut init.race_number, "race")?;

            let mut race: Race = dir_entry::read_json(&entry, "race")?;
            race.number = number;
            let room_number = race.room_number;
            let next_room_number = room_number.checked_add(1).context("too many rooms")?;
            init.room_number = init.room_number.max(next_room_number);
        }

        Ok((Self { batch_sender, players }, init))
    }

    pub fn read_player<R>(&self, player_id: &PlayerId, f: impl Fn(Option<&Player>) -> R) -> R {
        self.players.read_sync(player_id, |_, player| f(Some(player))).unwrap_or_else(|| f(None))
    }

    pub fn store(&self, players: Vec<Player, MAX_ROOM_PLAYER_COUNT>, race: Race) -> Result<()> {
        for player in &players {
            self.players.upsert_sync(player.id(), player.clone());
        }

        let batch = Batch { players, race };
        Ok(self.batch_sender.try_send(batch)?)
    }
}

fn update_number(entry: &DirEntry, init_number: &mut u64, name: &str) -> Result<u64> {
    let number: u64 = dir_entry::extract_json_stem(entry, name)?;
    let next_number = number.checked_add(1).with_context(|| format!("too many {name}s"))?;
    *init_number = (*init_number).max(next_number);
    Ok(number)
}
