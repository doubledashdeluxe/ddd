use std::collections::{HashMap, HashSet};
use std::net::{Ipv4Addr, SocketAddr};

use noise_protocol::DH;

use crate::crypto::PublicKey;
use crate::crypto::kx;
use crate::crypto::session::Session;
use crate::crypto::x25519::X25519;
use crate::flood::flood::Flood;
use crate::formats::online::*;
use crate::formats::version;

pub struct Identity {
    server_pk: PublicKey,
    addr: u32,
    port: u16,
    connections: HashSet<SocketAddr>,
    messages: Vec<([u8; 8 + kx::M1_SIZE], SocketAddr)>,
    client_states: HashMap<SocketAddr, kx::ClientState>,
    sessions: HashMap<SocketAddr, Session>,
    ops: u64,
}

impl Flood for Identity {
    fn new(server_pk: PublicKey, thread_index: usize) -> Self {
        Self {
            server_pk,
            addr: 0,
            port: thread_index as u16,
            messages: vec![],
            client_states: HashMap::new(),
            connections: HashSet::new(),
            sessions: HashMap::new(),
            ops: 0,
        }
    }

    fn read(&mut self, message: &[u8], addr: SocketAddr) {
        if message.len() == 8 {
            if !self.connections.insert(addr) {
                return;
            }
            let mut m = [0; _];
            m[..8].copy_from_slice(message);
            let client_k = X25519::genkey();
            let m1 = &mut m[8..];
            let client_state = kx::ik_1(client_k, self.server_pk, m1);
            self.messages.push((m, addr));
            self.client_states.insert(addr, client_state);
            return;
        }

        let session = if message.len() == kx::M2_SIZE {
            let Some(client_state) = self.client_states.remove(&addr) else { return };
            let Ok(session) = kx::ik_3(client_state, message) else { return };
            self.sessions.entry(addr).or_insert(session)
        } else {
            let Some(session) = self.sessions.get_mut(&addr) else { return };
            session
        };
        let unspecified = ClientIdentityUnspecified {};
        let server = ClientStateServer {
            update_version: UPDATE_VERSION,
            reserved: 0,
            protocol_version: PROTOCOL_VERSION,
            version: version::VERSION.as_bytes().try_into().unwrap(),
            client_identity: ClientIdentity::Unspecified(unspecified),
        };
        let client_state = ClientState::Server(server);
        let mut message = [0; 8 + kx::M1_SIZE - (Session::MAC_SIZE + Session::NONCE_SIZE)];
        client_state.write(&mut message).unwrap();
        let mut m = [0; _];
        session.encrypt(&message, &mut m);
        self.messages.push((m, addr));
        self.ops += 1;
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
