use std::collections::HashMap;

use crate::player::Name;
use crate::storage::PlayerId;
use crate::website::rankings::Rankings;
use crate::website::stats::Stats;

#[derive(Debug, Default)]
pub struct Init {
    pub player_numbers: HashMap<PlayerId, u64>,
    pub player_names: HashMap<u64, Name>,
    pub rankings: Rankings,
    pub stats: Stats,
}
