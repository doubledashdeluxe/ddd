pub use crate::player::name::Name;

use std::time::Duration;

use heapless::LinearMap;

use crate::formats::online::{MODE_INDEX_COUNT, ModeIndex, ServerPlayer};

mod name;

#[derive(Clone, Debug)]
pub struct Player {
    pub player: ServerPlayer,
    pub mmrs: LinearMap<ModeIndex, u16, MODE_INDEX_COUNT>,
    pub match_count: u64,
    pub play_time: Duration,
}

impl Player {
    pub const fn index(&self) -> u8 {
        self.player.index
    }

    pub const fn name(&self) -> Name {
        Name(self.player.name)
    }
}
