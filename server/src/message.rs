use std::net::SocketAddr;

use crate::buffer::Buffer;
use crate::formats::online::FrameRate;

pub enum Message {
    Read { buffer: Buffer, addr: SocketAddr },
    Write { frame_rate: FrameRate },
}
