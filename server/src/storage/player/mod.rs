pub use crate::storage::player::id::Id;

use std::time::Duration;

use heapless::LinearMap;
use jiff::fmt::serde::unsigned_duration::required;
use serde::{Deserialize, Serialize};

use crate::base64;
use crate::crypto::PublicKey;
use crate::formats::online::{MODE_INDEX_COUNT, ModeIndex};
use crate::player::Name;

mod id;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Player {
    #[serde(with = "base64")]
    pub client_pk: PublicKey,
    pub index: u8,
    pub name: Name,
    pub mmrs: LinearMap<ModeIndex, u16, MODE_INDEX_COUNT>,
    pub race_count: u64,
    #[serde(with = "required")]
    pub play_time: Duration,
}

impl Player {
    pub const fn id(&self) -> Id {
        Id { client_pk: self.client_pk, index: self.index }
    }
}
