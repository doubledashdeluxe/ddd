use noise_protocol::Cipher;
use orion::hazardous::aead::chacha20poly1305::{self, Nonce, SecretKey};

use crate::sensitive::Sensitive;

pub enum ChaCha20Poly1305 {}

impl Cipher for ChaCha20Poly1305 {
    type Key = Sensitive<[u8; 32]>;

    fn name() -> &'static str {
        "ChaChaPoly"
    }

    fn encrypt(k: &Self::Key, nonce: u64, ad: &[u8], plaintext: &[u8], out: &mut [u8]) {
        let k = SecretKey::from_slice(k.as_slice()).unwrap();
        let mut full_nonce = [0u8; 12];
        full_nonce[4..].copy_from_slice(&nonce.to_le_bytes());
        let full_nonce = Nonce::from(full_nonce);
        chacha20poly1305::seal(&k, &full_nonce, plaintext, Some(ad), out).unwrap();
    }

    fn encrypt_in_place(_: &Self::Key, _: u64, _: &[u8], _: &mut [u8], _: usize) -> usize {
        unimplemented!()
    }

    fn decrypt(
        k: &Self::Key,
        nonce: u64,
        ad: &[u8],
        ciphertext: &[u8],
        out: &mut [u8],
    ) -> Result<(), ()> {
        let k = SecretKey::from_slice(k.as_slice()).unwrap();
        let mut full_nonce = [0u8; 12];
        full_nonce[4..].copy_from_slice(&nonce.to_le_bytes());
        let full_nonce = Nonce::from(full_nonce);
        chacha20poly1305::open(&k, &full_nonce, &ciphertext, Some(ad), out).map_err(|_| ())
    }

    fn decrypt_in_place(
        _: &Self::Key,
        _: u64,
        _: &[u8],
        _: &mut [u8],
        _: usize,
    ) -> Result<usize, ()> {
        unimplemented!()
    }
}
