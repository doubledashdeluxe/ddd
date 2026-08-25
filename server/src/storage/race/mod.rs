pub use crate::storage::race::kart::Kart;
pub use crate::storage::race::player::Player;

use heapless::Vec;
use jiff::Timestamp;
use serde::{Deserialize, Serialize};

use crate::formats::online::*;
use crate::storage::base64;

mod kart;
mod player;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Race {
    #[serde(skip)]
    pub room_id: u128,
    pub room_number: u64,
    pub karts: Vec<Kart, MAX_ROOM_KART_COUNT>,
    pub spectator_count: u64,
    pub mode: ModeIndex,
    pub pack_course_count: usize,
    #[serde(with = "base64")]
    pub pack_hash: [u8; 32],
    pub code_type: RoomOptionCodeType,
    pub format: RoomOptionFormat,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub engine_size: Option<RoomOptionEngineSize>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub item_mode: Option<RoomOptionItemMode>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub lap_count: Option<u8>,
    pub race_count: u8,
    pub course_selection: RoomOptionCourseSelection,
    pub race_index: u8,
    pub start: Timestamp,
    pub selected_kart_index: u8,
    #[serde(with = "base64")]
    pub course_hash: [u8; 32],
    pub end: Timestamp,
}
