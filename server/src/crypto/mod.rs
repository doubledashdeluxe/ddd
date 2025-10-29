use sensitive::Sensitive;

pub mod blake2b;
pub mod chacha20poly1305;
pub mod kx;
pub mod sensitive;
pub mod session;
pub mod x25519;

pub type Key = Sensitive<[u8; 32]>;
pub type PublicKey = [u8; 32];
