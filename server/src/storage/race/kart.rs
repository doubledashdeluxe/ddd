use heapless::Vec;
use serde::{Deserialize, Serialize};

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::storage::race::player::Player;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Kart {
    #[serde(skip)]
    pub client_pk: PublicKey,
    pub players: Vec<Player, MAX_KART_PLAYER_COUNT>,
    pub points: u16,
    pub team: u8,
    pub poll_index: u8,
    pub characters: [CharacterId; 2],
    pub kart: KartId,
    pub course_index: u8,
    pub result_index: u8,
    pub result_time: u32,
    pub result_points: u16,
}
