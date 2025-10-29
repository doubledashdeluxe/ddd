use crate::crypto::PublicKey;
use crate::formats::online::ServerKart;

pub struct Kart {
    client_pk: PublicKey,
    server_kart: ServerKart,
}

impl Kart {
    pub fn new(client_pk: PublicKey, server_kart: ServerKart) -> Kart {
        Kart { client_pk, server_kart }
    }

    pub fn client_pk(&self) -> &PublicKey {
        &self.client_pk
    }

    pub fn server_kart(&self) -> &ServerKart {
        &self.server_kart
    }
}
