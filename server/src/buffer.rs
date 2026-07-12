use crate::formats::online::BUFFER_SIZE;

#[derive(Clone)]
pub struct Buffer {
    buffer: Box<[u8; MAX_LEN]>,
    len: usize,
}

impl Buffer {
    pub fn new() -> Self {
        Self { buffer: Box::new([0; MAX_LEN]), len: MAX_LEN }
    }

    pub fn as_slice(&self) -> &[u8] {
        &self.buffer[..self.len]
    }

    pub fn as_mut_slice(&mut self) -> &mut [u8] {
        &mut self.buffer[..self.len]
    }

    pub const fn set_len(&mut self, len: usize) {
        self.len = len;
    }

    pub const fn reset_len(&mut self) {
        self.set_len(MAX_LEN);
    }

    pub fn copy_from(&mut self, buffer: &[u8]) -> usize {
        let buffer = &buffer[..buffer.len().min(MAX_LEN)];
        self.set_len(buffer.len());
        self.as_mut_slice().copy_from_slice(buffer);
        self.len
    }
}

const MAX_LEN: usize = BUFFER_SIZE as usize;
