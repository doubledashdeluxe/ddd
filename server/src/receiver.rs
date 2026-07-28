use std::net::{SocketAddr, UdpSocket};

use anyhow::Result;

pub trait Receiver: Send + 'static {
    fn recv_from(&self, buf: &mut [u8]) -> Result<(usize, SocketAddr)>;
}

impl Receiver for UdpSocket {
    fn recv_from(&self, buf: &mut [u8]) -> Result<(usize, SocketAddr)> {
        Ok(self.recv_from(buf)?)
    }
}
