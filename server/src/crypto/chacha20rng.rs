use std::fmt::{self, Debug, Formatter};

use orion::hazardous::stream::chacha20::{self, Nonce, SecretKey};
use rand::rand_core::{CryptoRng, RngCore, SeedableRng, impls};
use zeroize::Zeroize;

pub struct ChaCha20Rng {
    buffer: [u8; BUFFERSIZE],
    offset: usize,
}

impl Debug for ChaCha20Rng {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        write!(f, "ChaCha20Rng {{}}")
    }
}

impl RngCore for ChaCha20Rng {
    fn next_u32(&mut self) -> u32 {
        impls::next_u32_via_fill(self)
    }

    fn next_u64(&mut self) -> u64 {
        impls::next_u64_via_fill(self)
    }

    fn fill_bytes(&mut self, mut dst: &mut [u8]) {
        let mut zeroize_offset = self.offset;

        while !dst.is_empty() {
            if self.offset == self.buffer.len() {
                let secret_key = &self.buffer[..chacha20::CHACHA_KEYSIZE];
                let secret_key = SecretKey::from_slice(secret_key).unwrap();
                let nonce = [0; chacha20::IETF_CHACHA_NONCESIZE];
                let nonce = Nonce::from_slice(&nonce).unwrap();
                let plaintext = [0; BUFFERSIZE];
                chacha20::encrypt(&secret_key, &nonce, 0, &plaintext, &mut self.buffer).unwrap();
                self.offset = chacha20::CHACHA_KEYSIZE;
                zeroize_offset = self.offset;
            }

            let len = dst.len().min(self.buffer.len() - self.offset);
            dst[..len].copy_from_slice(&self.buffer[self.offset..self.offset + len]);
            dst = &mut dst[len..];
            self.offset += len;
        }

        self.buffer[zeroize_offset..self.offset].zeroize();
    }
}

impl CryptoRng for ChaCha20Rng {}

impl SeedableRng for ChaCha20Rng {
    type Seed = [u8; chacha20::CHACHA_KEYSIZE];

    fn from_seed(seed: Self::Seed) -> Self {
        let mut buffer = [0; BUFFERSIZE];
        buffer[..seed.len()].copy_from_slice(&seed);
        Self { buffer, offset: BUFFERSIZE }
    }
}

const BUFFERSIZE: usize = 256;
