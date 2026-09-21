use heapless::Vec;
use serde::{Deserialize, Serialize};

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::kart::{Platform, Region};
use crate::storage::race::player::Player;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Kart {
    #[serde(skip)]
    pub client_pk: PublicKey,
    #[serde(default, skip_serializing_if = "Region::is_unknown")]
    pub region: Region,
    #[serde(default, skip_serializing_if = "Platform::is_empty")]
    pub platform: Platform,
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
    #[serde(skip_serializing_if = "Option::is_none")]
    pub delayed_frames: Option<u64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub latency: Option<u64>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub stability: Option<u64>,
}
