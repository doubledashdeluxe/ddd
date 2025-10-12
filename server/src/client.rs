use std::mem;
use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::Result;

use crate::formats::client_state::{
    ClientIdentity, ClientIdentitySpecified, ClientState, ClientStatePack,
};
use crate::formats::server_state::{
    ServerIdentity, ServerIdentitySpecified, ServerIdentityUnspecified, ServerMode, ServerState,
    ServerStateMode, ServerStatePack, ServerStateServer,
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
        let identity = match &self.state {
            State::Mode { identity } => identity,
            State::Pack { identity, .. } => identity,
            _ => return 0,
        };
        identity.players.len()
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

        let client_state = self.client_state.take();
        if let Some(client_state) = client_state {
            let state = mem::replace(&mut self.state, State::Idle);
            let identity = match state {
                State::Idle => None,
                State::Server { identity } => identity,
                State::Mode { identity } => Some(identity),
                State::Pack { identity, .. } => Some(identity),
            };
            match (client_state, identity) {
                (ClientState::Server(server), _) => {
                    let identity = match server.client_identity {
                        ClientIdentity::Unspecified(_) => None,
                        ClientIdentity::Specified(identity) => {
                            anyhow::ensure!(server.protocol_version == version::PROTOCOL_VERSION);
                            Some(identity)
                        }
                    };
                    self.state = State::Server { identity };
                }
                (ClientState::Mode(_), Some(identity)) => self.state = State::Mode { identity },
                (ClientState::Pack(pack), Some(identity)) => {
                    self.state = State::Pack { identity, pack };
                }
                _ => anyhow::bail!("Unexpected client state"),
            }
        }

        let server_state = match &self.state {
            State::Idle => return Ok(None),
            State::Server { identity } => {
                let protocol_version = version::PROTOCOL_VERSION;
                let major = version::MAJOR_VERSION;
                let minor = version::MINOR_VERSION;
                let patch = version::PATCH_VERSION;
                let version = format!("v{major}.{minor}.{patch}").into();
                let server_identity = if identity.is_some() {
                    let motd = "test motd".into();
                    let player_count = player_count as u16;
                    let identity = ServerIdentitySpecified { motd, player_count };
                    ServerIdentity::Specified(identity)
                } else {
                    let identity = ServerIdentityUnspecified {};
                    ServerIdentity::Unspecified(identity)
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
            State::Pack { pack, .. } => {
                let ClientStatePack { mode_index, pack_index, .. } = *pack;
                let player_count = pack_index as u16 * 137;
                let pack = ServerStatePack { mode_index, pack_index, player_count };
                ServerState::Pack(pack)
            }
        };
        let message_len = message.len() - server_state.write(message).unwrap().len();
        Ok(Some(message_len))
    }
}

enum State {
    Idle,
    Server { identity: Option<ClientIdentitySpecified> },
    Mode { identity: ClientIdentitySpecified },
    Pack { identity: ClientIdentitySpecified, pack: ClientStatePack },
}
