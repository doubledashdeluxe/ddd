use anyhow::Result;
use noise_protocol::Cipher;

use crate::crypto::chacha20poly1305::ChaCha20Poly1305;
use crate::crypto::sensitive::Sensitive;

pub struct Session {
    read_k: Sensitive<[u8; 32]>,
    read_nonce: u64,
    write_k: Sensitive<[u8; 32]>,
    write_nonce: u64,
}

impl Session {
    pub const MAC_SIZE: usize = 16;
    pub const NONCE_SIZE: usize = 8;

    pub fn new(read_k: Sensitive<[u8; 32]>, write_k: Sensitive<[u8; 32]>) -> Session {
        let read_nonce = 0;
        let write_nonce = 0;
        Session { read_k, read_nonce, write_k, write_nonce }
    }

    #[cfg(test)]
    pub fn read_k(&self) -> &Sensitive<[u8; 32]> {
        &self.read_k
    }

    #[cfg(test)]
    pub fn write_k(&self) -> &Sensitive<[u8; 32]> {
        &self.write_k
    }

    pub fn decrypt(&mut self, ciphertext: &[u8], plaintext: &mut [u8]) -> Result<()> {
        let (ciphertext, nonce) = ciphertext.split_last_chunk().unwrap();
        let nonce = u64::from_le_bytes(*nonce);
        anyhow::ensure!(nonce >= self.read_nonce && nonce != u64::MAX);
        ChaCha20Poly1305::decrypt(&self.read_k, nonce, &[], ciphertext, plaintext)
            .map_err(|_| anyhow::anyhow!("Decryption failed"))?;
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
