use std::net::{Ipv4Addr, SocketAddr};

use crate::crypto::PublicKey;
use crate::flood::flood::Flood;

pub struct Cookie {
    port: u16,
    ops: u64,
}

impl Flood for Cookie {
    fn new(_: PublicKey, _: usize) -> Self {
        Self { port: 0, ops: 0 }
    }

    fn read(&mut self, _: &[u8], _: SocketAddr) {
        self.ops += 1;
    }

    fn write(&mut self, _: &mut [u8]) -> (usize, SocketAddr) {
        let addr = (Ipv4Addr::UNSPECIFIED, self.port).into();
        self.port = self.port.wrapping_add(1);
        (8, addr)
    }

    fn ops(&self) -> u64 {
        self.ops
    }
}
