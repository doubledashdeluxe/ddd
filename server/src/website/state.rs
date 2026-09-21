use std::collections::HashMap;

use crate::player::Name;
use crate::storage::PlayerId;
use crate::website::rankings::Rankings;
use crate::website::stats::Stats;

pub struct State {
    pub player_numbers: HashMap<PlayerId, u64>,
    pub player_names: HashMap<u64, Name>,
    pub room_numbers: HashMap<u128, u64>,
    pub rankings: Rankings,
    pub stats: Stats,
    pub counter: u64,
}
