use crate::crypto::PublicKey;
use crate::formats::online::ServerPlayer;

pub struct Kart {
    client_pk: PublicKey,
    players: Vec<ServerPlayer>,
}

impl Kart {
    pub fn new(client_pk: PublicKey, players: Vec<ServerPlayer>) -> Kart {
        Kart { client_pk, players }
    }

    pub fn client_pk(&self) -> &PublicKey {
        &self.client_pk
    }

    pub fn players(&self) -> &[ServerPlayer] {
        &self.players
    }
}
