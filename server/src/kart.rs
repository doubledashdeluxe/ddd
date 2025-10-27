use crate::formats::online::ServerKart;

pub struct Kart {
    client_pk: [u8; 32],
    server_kart: ServerKart,
}

impl Kart {
    pub fn new(client_pk: [u8; 32], server_kart: ServerKart) -> Kart {
        Kart { client_pk, server_kart }
    }

    pub fn client_pk(&self) -> &[u8; 32] {
        &self.client_pk
    }

    pub fn server_kart(&self) -> &ServerKart {
        &self.server_kart
    }
}
