use serde::{Deserialize, Serialize};

use crate::player::Name;

#[derive(Clone, Copy, Debug, Serialize, Deserialize)]
pub struct Player {
    #[serde(skip)]
    pub profile: u8,
    pub number: u64,
    pub name: Name,
    pub mmr: u16,
}
