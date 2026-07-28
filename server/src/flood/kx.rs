use std::net::{Ipv4Addr, SocketAddr};

use crate::crypto::PublicKey;
use crate::crypto::kx;
use crate::flood::flood::Flood;

pub struct Kx {
    port: u16,
    cookie: Option<[u8; 8]>,
    ops: u64,
}

impl Flood for Kx {
    fn new(_: PublicKey, _: usize) -> Self {
        Self { port: 0, cookie: None, ops: 0 }
    }

    fn read(&mut self, message: &[u8], addr: SocketAddr) {
        if message.len() == 8 {
            self.port = addr.port();
            self.cookie.get_or_insert_default().copy_from_slice(message);
        }
    }

    fn write(&mut self, message: &mut [u8]) -> (usize, SocketAddr) {
        let addr = (Ipv4Addr::UNSPECIFIED, self.port).into();
        match self.cookie {
            Some(cookie) => {
                message[..8].copy_from_slice(&cookie);
                self.ops += 1;
            }
            None => self.port = self.port.wrapping_add(1),
        }
        (8 + kx::M1_SIZE, addr)
    }

    fn ops(&self) -> u64 {
        self.ops
    }
}
