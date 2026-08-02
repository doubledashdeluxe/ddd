use crate::crypto::PublicKey;
use crate::formats::online::MAX_KART_PLAYER_COUNT;
use crate::mmr::Mmr;
use crate::player::Player;

use heapless::Vec;

#[derive(Clone, Debug)]
pub struct Kart {
    client_pk: PublicKey,
    players: Vec<Player, MAX_KART_PLAYER_COUNT>,
    pub points: u16,
}

impl Kart {
    pub const fn new(client_pk: PublicKey, players: Vec<Player, MAX_KART_PLAYER_COUNT>) -> Self {
        Self { client_pk, players, points: 0 }
    }

    pub const fn client_pk(&self) -> &PublicKey {
        &self.client_pk
    }

    pub fn players(&self) -> &[Player] {
        &self.players
    }
}

impl Mmr for Kart {
    fn mmr(&self) -> u16 {
        self.players.mmr()
    }
}
