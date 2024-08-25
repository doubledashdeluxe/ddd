use noise_protocol::{Hash, U8Array};
use orion::hazardous::hash::blake2::blake2b;

use crate::sensitive::Sensitive;

pub struct Blake2b(blake2b::Blake2b);

impl Default for Blake2b {
    fn default() -> Self {
        Blake2b(blake2b::Blake2b::new(64).unwrap())
    }
}

impl Hash for Blake2b {
    type Block = [u8; 128];
    type Output = Sensitive<[u8; 64]>;

    fn name() -> &'static str {
        "BLAKE2b"
    }

    fn input(&mut self, data: &[u8]) {
        self.0.update(data).unwrap();
    }

    fn result(&mut self) -> Self::Output {
        Self::Output::from_slice(self.0.finalize().unwrap().as_ref())
    }
}
