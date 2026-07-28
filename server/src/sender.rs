use std::net::{SocketAddr, UdpSocket};

use anyhow::Result;

pub trait Sender: Send + 'static {
    fn send_to(&self, buf: &[u8], addr: SocketAddr) -> Result<usize>;
}

impl Sender for UdpSocket {
    fn send_to(&self, buf: &[u8], addr: SocketAddr) -> Result<usize> {
        Ok(self.send_to(buf, addr)?)
    }
}
