use crate::crypto::PublicKey;

#[derive(Clone, Copy, Debug, Eq, Hash, PartialEq)]
pub struct Id {
    pub client_pk: PublicKey,
    pub index: u8,
}
