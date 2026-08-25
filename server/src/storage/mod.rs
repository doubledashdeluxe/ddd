pub use crate::storage::player::Id as PlayerId;
pub use crate::storage::player::Player;
pub use crate::storage::race::Race;

use std::ffi::OsStr;
use std::fs::{self, DirEntry};
use std::path::Path;
use std::sync::mpsc::SyncSender;

use anyhow::{Context, Result, anyhow};
use heapless::Vec;
use scc::HashMap;
use serde::de::DeserializeOwned;

use crate::formats::online::*;
use crate::storage::batch::Batch;
use crate::storage::init::Init;

pub mod race;
pub mod worker;

mod base64;
mod batch;
mod init;
mod player;

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

            let player: Player = read(&entry)?;
            let id = player.id();
            players.insert_sync(id, player).map_err(|_| anyhow!("duplicate player {id:?}"))?;

            init.player_numbers.insert(id, number);
        }

        let races_path = path.join("races");
        fs::create_dir_all(&races_path)?;
        for entry in fs::read_dir(&races_path)? {
            let entry = entry?;

            update_number(&entry, &mut init.race_number, "race")?;

            let race: Race = read(&entry)?;
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
    let file_name = entry.file_name();
    let number = extract_number(&file_name)
        .with_context(|| format!("invalid {name} file name {}", file_name.display()))?;
    let next_number = number.checked_add(1).with_context(|| format!("too many {name}s"))?;
    *init_number = (*init_number).max(next_number);
    Ok(number)
}

fn extract_number(file_name: &OsStr) -> Result<u64> {
    let file_name = file_name.to_str().context("invalid UTF-8")?;
    let number = file_name.strip_suffix(".json").context("missing .json extension")?;
    Ok(number.parse()?)
}

fn read<T: DeserializeOwned>(entry: &DirEntry) -> Result<T> {
    let path = entry.path();
    let x = fs::read_to_string(path)?;
    Ok(serde_json::from_str(&x)?)
}
