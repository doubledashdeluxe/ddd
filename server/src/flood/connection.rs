use std::net::{Ipv4Addr, SocketAddr};

use noise_protocol::DH;

use crate::crypto::PublicKey;
use crate::crypto::kx;
use crate::crypto::x25519::X25519;
use crate::flood::flood::Flood;

pub struct Connection {
    server_pk: PublicKey,
    addr: u32,
    port: u16,
    messages: Vec<([u8; 8 + kx::M1_SIZE], SocketAddr)>,
    ops: u64,
}

impl Flood for Connection {
    fn new(server_pk: PublicKey, thread_index: usize) -> Self {
        Self { server_pk, addr: 0, port: thread_index as u16, messages: vec![], ops: 0 }
    }

    fn read(&mut self, message: &[u8], addr: SocketAddr) {
        if message.len() == 8 {
            let mut m = [0; _];
            m[..8].copy_from_slice(message);
            let client_k = X25519::genkey();
            let m1 = &mut m[8..];
            kx::ik_1(client_k, self.server_pk, m1);
            self.messages.push((m, addr));
            self.ops += 1;
        }
    }

    fn write(&mut self, message: &mut [u8]) -> (usize, SocketAddr) {
        if let Some((m, addr)) = self.messages.pop() {
            message[..m.len()].copy_from_slice(&m);
            (m.len(), addr)
        } else {
            self.addr = self.addr.wrapping_add(1);
            let addr: Ipv4Addr = self.addr.into();
            let addr = (addr, self.port).into();
            (8, addr)
        }
    }

    fn ops(&self) -> u64 {
        self.ops
    }
}
