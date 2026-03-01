pub struct Buffer {
    buffer: Box<[u8; MAX_LEN]>,
    len: usize,
}

impl Buffer {
    pub fn new() -> Self {
        Self { buffer: Box::new([0; MAX_LEN]), len: MAX_LEN }
    }

    pub fn as_slice(&self) -> &[u8] {
        &self.buffer[0..self.len]
    }

    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        &mut self.buffer[0..self.len]
    }

    pub const fn set_len(&mut self, len: usize) {
        self.len = len;
    }

    pub const fn reset_len(&mut self) {
        self.set_len(MAX_LEN);
    }
}

const MAX_LEN: usize = 512;
