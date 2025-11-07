use std::net::SocketAddr;

use crate::buffer::Buffer;

pub enum Message {
    Read { addr: SocketAddr, buffer: Buffer },
    Write,
    Stop,
}
