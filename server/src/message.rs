use std::net::SocketAddr;

use crate::buffer::Buffer;
use crate::formats::online::FrameRate;

pub enum Message {
    Read { addr: SocketAddr, buffer: Buffer },
    Write { frame_rate: FrameRate },
    Stop,
}
