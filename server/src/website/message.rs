use heapless::Vec;

use crate::formats::online::*;
use crate::storage::{Player, Race, Stats};

#[derive(Debug)]
pub enum Message {
    Batch(Batch),
    Stats(Stats),
}

#[derive(Debug)]
pub struct Batch {
    pub players: Vec<Player, MAX_ROOM_PLAYER_COUNT>,
    pub race: Race,
}
