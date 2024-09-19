use noise_protocol::{U8Array, DH};
use orion::hazardous::ecc::x25519::{self, PrivateKey, PublicKey};

use crate::crypto::sensitive::Sensitive;

pub enum X25519 {}

impl DH for X25519 {
    type Key = Sensitive<[u8; 32]>;
    type Pubkey = [u8; 32];
    type Output = Sensitive<[u8; 32]>;

    fn name() -> &'static str {
        "25519"
    }

    fn genkey() -> Self::Key {
        let k = PrivateKey::generate();
        Self::Key::from_slice(k.unprotected_as_bytes())
    }

    fn pubkey(k: &Self::Key) -> Self::Pubkey {
        let k = PrivateKey::from_slice(k.as_slice()).unwrap();
        let pk = PublicKey::try_from(&k).unwrap();
        pk.to_bytes()
    }

    fn dh(k: &Self::Key, pk: &Self::Pubkey) -> Result<Self::Output, ()> {
        let k = PrivateKey::from_slice(k.as_slice()).unwrap();
        let pk = PublicKey::from(*pk);
        let sk = x25519::key_agreement(&k, &pk).map_err(|_| ())?;
        Ok(Self::Output::from_slice(sk.unprotected_as_bytes()))
    }
}
