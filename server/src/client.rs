use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::Result;

use crate::formats::client_state::{ClientIdentity, ClientState, ClientStateServer};
use crate::formats::server_state::{
    ServerIdentity, ServerIdentitySpecified, ServerIdentityUnspecified, ServerState,
    ServerStateServer, ServerVersion,
};

pub struct Client {
    expiration: Instant,
    addr: SocketAddr,
    state: State,
}

impl Client {
    pub fn new(now: Instant, addr: SocketAddr) -> Client {
        let expiration = now + Duration::from_secs(120);
        let state = State::Idle;
        Client { expiration, addr, state }
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
        match client_state {
            ClientState::Server(server) => {
                if let ClientIdentity::Specified(_) = server.client_identity {
                    let protocol_version = 1;
                    anyhow::ensure!(server.protocol_version == protocol_version);
                }
                self.state = State::Server { server };
            }
            _ => anyhow::bail!("Unexpected client state"),
        }
        self.expiration = now + Duration::from_secs(120);
        Ok(())
    }

    pub fn write(
        &mut self,
        now: Instant,
        addr: SocketAddr,
        message: &mut [u8],
    ) -> Result<Option<usize>> {
        anyhow::ensure!(now < self.expiration);
        anyhow::ensure!(addr == self.addr);
        let server_state = match &self.state {
            State::Idle => return Ok(None),
            State::Server { server } => {
                let protocol_version = 1;
                let server_version = ServerVersion { major: 2, minor: 3, patch: 4 };
                let server_identity = match server.client_identity {
                    ClientIdentity::Unspecified(_) => {
                        let server_identity_unspecified = ServerIdentityUnspecified {};
                        ServerIdentity::Unspecified(server_identity_unspecified)
                    }
                    ClientIdentity::Specified(_) => {
                        let motd = "test motd".into();
                        let player_count = 567;
                        let server_identity_specified =
                            ServerIdentitySpecified { motd, player_count };
                        ServerIdentity::Specified(server_identity_specified)
                    }
                };
                let server_state_server =
                    ServerStateServer { protocol_version, server_version, server_identity };
                ServerState::Server(server_state_server)
            }
        };
        let message_len = message.len() - server_state.write(message).unwrap().len();
        Ok(Some(message_len))
    }
}

enum State {
    Idle,
    Server { server: ClientStateServer },
}
