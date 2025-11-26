use crate::crypto::PublicKey;
use crate::mmr::Mmr;
use crate::player::Player;

pub struct Kart {
    client_pk: PublicKey,
    players: Vec<Player>,
}

impl Kart {
    pub fn new(client_pk: PublicKey, players: Vec<Player>) -> Kart {
        Kart { client_pk, players }
    }

    pub fn client_pk(&self) -> &PublicKey {
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
