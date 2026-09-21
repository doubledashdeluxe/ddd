use jiff::civil::DateTime;
use serde::{Deserialize, Serialize};

#[derive(Clone, Copy, Debug, Serialize, Deserialize)]
pub struct Stats {
    #[serde(skip)]
    pub dt: DateTime,
    pub player_count: u64,
    pub room_count: u64,
}
