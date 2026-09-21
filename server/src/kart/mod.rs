pub use crate::kart::platform::Platform;
pub use crate::kart::region::Region;

use heapless::Vec;

use crate::crypto::PublicKey;
use crate::formats::online::{MAX_KART_PLAYER_COUNT, ModeIndex};
use crate::mmr;
use crate::player::Player;

mod platform;
mod region;

#[derive(Clone, Debug)]
pub struct Kart {
    client_pk: PublicKey,
    region: Region,
    platform: Platform,
    players: Vec<Player, MAX_KART_PLAYER_COUNT>,
    pub points: u16,
}

impl Kart {
    pub const fn new(
        client_pk: PublicKey,
        region: Region,
        platform: Platform,
        players: Vec<Player, MAX_KART_PLAYER_COUNT>,
    ) -> Self {
        Self { client_pk, region, platform, players, points: 0 }
    }

    pub const fn client_pk(&self) -> &PublicKey {
        &self.client_pk
    }

    pub const fn region(&self) -> Region {
        self.region
    }

    pub const fn platform(&self) -> &Platform {
        &self.platform
    }

    pub fn players(&self) -> &[Player] {
        &self.players
    }

    pub fn players_mut(&mut self) -> &mut [Player] {
        &mut self.players
    }

    pub fn mmr(&self, mode_index: ModeIndex) -> u16 {
        mmr::mmr(self.players.iter().map(|player| *player.mmrs.get(&mode_index).unwrap_or(&0)))
    }
}
