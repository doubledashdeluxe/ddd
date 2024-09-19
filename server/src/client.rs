use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::Result;

use crate::formats::client_state::ClientState;
use crate::formats::server_state::{
    ServerIdentity, ServerIdentityUnspecified, ServerState, ServerStateServer, ServerVersion,
};

pub struct Client {
    expiration: Instant,
    addr: SocketAddr,
}

impl Client {
    pub fn new(now: Instant, addr: SocketAddr) -> Client {
        let expiration = now + Duration::from_secs(120);
        Client { expiration, addr }
    }

    pub fn has_expired(&self, now: Instant) -> bool {
        now >= self.expiration
    }

    pub fn set_addr(&mut self, addr: SocketAddr) {
        self.addr = addr;
    }

    pub fn read(&mut self, now: Instant, addr: SocketAddr, message: &[u8]) -> Result<()> {
        anyhow::ensure!(addr == self.addr);
        let (client_state, _) =
            ClientState::read(message).map_err(|_| anyhow::anyhow!("Invalid client state"))?;
        eprintln!("{:?}", client_state);
        self.expiration = now + Duration::from_secs(120);
        Ok(())
    }

    pub fn write(&mut self, now: Instant, addr: SocketAddr, message: &mut [u8]) -> Result<usize> {
        anyhow::ensure!(addr == self.addr);
        self.expiration = now + Duration::from_secs(120);
        let protocol_version = 1;
        let server_version = ServerVersion { major: 2, minor: 3, patch: 4 };
        let server_identity_unspecified = ServerIdentityUnspecified {};
        let server_identity = ServerIdentity::Unspecified(server_identity_unspecified);
        let server_state_server =
            ServerStateServer { protocol_version, server_version, server_identity };
        let server_state = ServerState::Server(server_state_server);
        let message_len = message.len() - server_state.write(message).unwrap().len();
        Ok(message_len)
    }
}
