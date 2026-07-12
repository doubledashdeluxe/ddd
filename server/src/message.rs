use std::net::SocketAddr;

use crate::buffer::Buffer;
use crate::frequency::Frequency;

pub enum Message {
    Read { buffer: Buffer, addr: SocketAddr },
    Write { frequency: Frequency, client_slots: usize },
}
