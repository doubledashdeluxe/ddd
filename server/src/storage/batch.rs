use heapless::Vec;

use crate::formats::online::*;
use crate::storage::player::Player;
use crate::storage::race::Race;

#[derive(Debug)]
pub struct Batch {
    pub players: Vec<Player, MAX_ROOM_PLAYER_COUNT>,
    pub race: Race,
}
