use std::mem;
use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::Result;

use crate::formats::client_state::{
    ClientIdentity, ClientIdentitySpecified, ClientState, ClientStateServer,
};
use crate::formats::server_state::{
    ServerIdentity, ServerIdentitySpecified, ServerIdentityUnspecified, ServerMode, ServerState,
    ServerStateMode, ServerStateServer,
};
use crate::formats::version;

pub struct Client {
    expiration: Instant,
    addr: SocketAddr,
    state: State,
    client_state: Option<ClientState>,
}

impl Client {
    pub fn new(now: Instant, addr: SocketAddr) -> Client {
        let expiration = now + Duration::from_secs(120);
        let state = State::Idle;
        let client_state = None;
        Client { expiration, addr, state, client_state }
    }

    pub fn has_expired(&self, now: Instant) -> bool {
        now >= self.expiration
    }

    pub fn set_addr(&mut self, addr: SocketAddr) {
        self.addr = addr;
    }

    pub fn player_count(&self) -> usize {
        match &self.state {
            State::Mode { identity } => identity.players.len(),
            _ => 0,
        }
    }

    pub fn read(&mut self, now: Instant, addr: SocketAddr, message: &[u8]) -> Result<()> {
        anyhow::ensure!(addr == self.addr);
        let (client_state, _) =
            ClientState::read(message).map_err(|_| anyhow::anyhow!("Invalid client state"))?;
        self.client_state = Some(client_state);
        self.expiration = now + Duration::from_secs(120);
        Ok(())
    }

    pub fn write(
        &mut self,
        addr: SocketAddr,
        message: &mut [u8],
        player_count: usize,
    ) -> Result<Option<usize>> {
        anyhow::ensure!(addr == self.addr);

        let state = mem::replace(&mut self.state, State::Idle);
        let client_state = self.client_state.take();
        match (client_state, state) {
            (Some(ClientState::Server(server)), _) => {
                if let ClientIdentity::Specified(_) = &server.client_identity {
                    anyhow::ensure!(server.protocol_version == version::PROTOCOL_VERSION);
                }
                self.state = State::Server { server };
            }
            (Some(ClientState::Mode(_)), State::Server { server }) => {
                let identity = match server.client_identity {
                    ClientIdentity::Specified(identity) => identity,
                    _ => anyhow::bail!("Unexpected client state"),
                };
                self.state = State::Mode { identity };
            }
            (Some(ClientState::Mode(_)), state @ State::Mode { .. }) => self.state = state,
            (None, state) => self.state = state,
            _ => anyhow::bail!("Unexpected client state"),
        }

        let server_state = match &self.state {
            State::Idle => return Ok(None),
            State::Server { server } => {
                let protocol_version = version::PROTOCOL_VERSION;
                let major = version::MAJOR_VERSION;
                let minor = version::MINOR_VERSION;
                let patch = version::PATCH_VERSION;
                let version = format!("v{major}.{minor}.{patch}").into();
                let server_identity = match server.client_identity {
                    ClientIdentity::Unspecified(_) => {
                        let identity = ServerIdentityUnspecified {};
                        ServerIdentity::Unspecified(identity)
                    }
                    ClientIdentity::Specified(_) => {
                        let motd = "test motd".into();
                        let player_count = player_count as u16;
                        let identity = ServerIdentitySpecified { motd, player_count };
                        ServerIdentity::Specified(identity)
                    }
                };
                let server_state_server =
                    ServerStateServer { protocol_version, version, server_identity };
                ServerState::Server(server_state_server)
            }
            State::Mode { identity } => {
                let modes = (0..5)
                    .map(|i| {
                        let mmrs =
                            (0..identity.players.len()).map(|j| (i * 4 + j as u16) * 179).collect();
                        let player_count = i * 79;
                        ServerMode { mmrs, player_count }
                    })
                    .collect();
                let mode = ServerStateMode { modes };
                ServerState::Mode(mode)
            }
        };
        let message_len = message.len() - server_state.write(message).unwrap().len();
        Ok(Some(message_len))
    }
}

enum State {
    Idle,
    Server { server: ClientStateServer },
    Mode { identity: ClientIdentitySpecified },
}
