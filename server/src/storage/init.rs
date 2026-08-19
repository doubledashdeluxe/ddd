use std::collections::HashMap;
use std::path::PathBuf;

use crate::storage::player::Id as PlayerId;

#[derive(Debug)]
pub struct Init {
    pub path: PathBuf,
    pub tmp_path: PathBuf,
    pub player_numbers: HashMap<PlayerId, u64>,
    pub player_number: u64,
    pub room_number: u64,
    pub race_number: u64,
}

impl Init {
    pub fn new(path: PathBuf, tmp_path: PathBuf) -> Self {
        Self {
            path,
            tmp_path,
            player_numbers: HashMap::new(),
            player_number: 0,
            room_number: 0,
            race_number: 0,
        }
    }
}
