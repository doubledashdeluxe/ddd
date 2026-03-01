use anyhow::Result;
use noise_protocol::Cipher;

use crate::crypto::Key;
use crate::crypto::chacha20poly1305::ChaCha20Poly1305;

pub struct Session {
    read_k: Key,
    read_nonce: u64,
    write_k: Key,
    write_nonce: u64,
}

impl Session {
    pub const MAC_SIZE: usize = 16;
    pub const NONCE_SIZE: usize = 8;

    pub const fn new(read_k: Key, write_k: Key) -> Self {
        let read_nonce = 0;
        let write_nonce = 0;
        Self { read_k, read_nonce, write_k, write_nonce }
    }

    #[cfg(test)]
    pub const fn read_k(&self) -> &Key {
        &self.read_k
    }

    #[cfg(test)]
    pub const fn write_k(&self) -> &Key {
        &self.write_k
    }

    pub fn decrypt(&mut self, ciphertext: &[u8], plaintext: &mut [u8]) -> Result<()> {
        let (ciphertext, nonce) = ciphertext.split_last_chunk().unwrap();
        let nonce = u64::from_le_bytes(*nonce);
        anyhow::ensure!(nonce >= self.read_nonce && nonce != u64::MAX);
        ChaCha20Poly1305::decrypt(&self.read_k, nonce, &[], ciphertext, plaintext)
            .map_err(|()| anyhow::anyhow!("Decryption failed"))?;
        self.read_nonce = nonce + 1;
        Ok(())
    }

    pub fn encrypt(&mut self, plaintext: &[u8], ciphertext: &mut [u8]) {
        let (ciphertext, nonce) = ciphertext.split_last_chunk_mut().unwrap();
        *nonce = self.write_nonce.to_le_bytes();
        ChaCha20Poly1305::encrypt(&self.write_k, self.write_nonce, &[], plaintext, ciphertext);
        self.write_nonce += 1;
    }
}
