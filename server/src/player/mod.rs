pub use crate::player::name::Name;

use std::time::Duration;

use heapless::LinearMap;

use crate::formats::online::{MODE_INDEX_COUNT, ModeIndex, ServerPlayer};

mod name;

#[derive(Clone, Debug)]
pub struct Player {
    pub profile: u8,
    pub index: u8,
    pub name: Name,
    pub mmrs: LinearMap<ModeIndex, u16, MODE_INDEX_COUNT>,
    pub match_count: u64,
    pub play_time: Duration,
}

impl Player {
    pub fn player(&self) -> ServerPlayer {
        ServerPlayer { index: self.index, name: self.name.into() }
    }
}
