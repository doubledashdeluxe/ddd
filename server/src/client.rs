use std::mem;
use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::{Result, anyhow};
use log::debug;

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::formats::version;
use crate::kart::Kart;
use crate::rooms::Rooms;

pub struct Client {
    expiration: Instant,
    addr: SocketAddr,
    pk: PublicKey,
    state: State,
    client_state: Option<ClientState>,
}

impl Client {
    pub fn new(now: Instant, addr: SocketAddr, pk: PublicKey) -> Client {
        debug!("-> {addr}");
        let expiration = now + Duration::from_secs(120);
        let state = State::Idle;
        let client_state = None;
        Client { expiration, addr, pk, state, client_state }
    }

    pub fn set_addr(&mut self, addr: SocketAddr) {
        debug!("<> {} {addr}", self.addr);
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

    pub fn room_id(&self) -> Option<u128> {
        match &self.state {
            State::Room { room_info: Some(room_info), .. } => Some(room_info.id),
            _ => None,
        }
    }

    pub fn update(&mut self, now: Instant, rooms: &mut Rooms) -> Result<()> {
        anyhow::ensure!(now < self.expiration);
        let client_state = self.client_state.take();
        let Some(client_state) = client_state else {
            return Ok(());
        };
        let state = mem::replace(&mut self.state, State::Idle);
        let (identity, room_info) = match state {
            State::Idle => (None, None),
            State::Server { identity } => (identity, None),
            State::Mode { identity } => (Some(identity), None),
            State::Pack { identity, .. } => (Some(identity), None),
            State::Room { identity, room_info } => (Some(identity), room_info),
        };
        self.state = match (client_state, identity) {
            (ClientState::Server(server), _) => {
                let identity = match server.client_identity {
                    ClientIdentity::Unspecified(_) => None,
                    ClientIdentity::Specified(identity) => {
                        anyhow::ensure!(server.protocol_version == PROTOCOL_VERSION);
                        let player_count = identity.players.len();
                        let kart_count = identity.kart_count as usize;
                        anyhow::ensure!(kart_count >= player_count.div_ceil(2));
                        anyhow::ensure!(kart_count <= player_count);
                        Some(identity)
                    }
                };
                State::Server { identity }
            }
            (ClientState::Mode(_), Some(identity)) => State::Mode { identity },
            (ClientState::Pack(pack), Some(identity)) => State::Pack { identity, pack },
            (ClientState::Room(room), Some(identity)) => {
                let karts = || {
                    let kart_count = identity.kart_count as usize;
                    let karts: Vec<_> = (0..kart_count)
                        .map(|i| {
                            let players = &identity.players;
                            let tandem_count = players.len() - kart_count;

                            let player = |i| {
                                let player: &ClientPlayer = &players[i];
                                let name = player.name.clone();
                                ServerPlayer { name }
                            };

                            let players = if i < tandem_count {
                                vec![player(i / 2 + 0), player(i / 2 + 1)]
                            } else {
                                vec![player(i + tandem_count)]
                            };
                            let kart = ServerKart { players };
                            Kart::new(self.pk, kart)
                        })
                        .collect();
                    karts
                };

                let room_info = match (room.client_room_state, room_info) {
                    (ClientRoomState::New(new), Some(room_info))
                        if room_info.counter == new.room_counter =>
                    {
                        rooms.get(&room_info.id).map(|_| room_info)
                    }
                    (ClientRoomState::New(new), _) => {
                        let counter = new.room_counter;
                        let karts = karts();
                        let mode_index = new.mode_index;
                        let pack_hash = new.pack_hash;
                        rooms.insert(karts, mode_index, pack_hash).map(|id| {
                            let spectating_counter = 0;
                            let spectating = false;
                            RoomInfo { counter, id, spectating_counter, spectating }
                        })
                    }
                    (ClientRoomState::Code(code), Some(room_info))
                        if code.room_counter == room_info.counter =>
                    {
                        rooms.get(&room_info.id).map(|_| room_info)
                    }
                    (ClientRoomState::Code(code), _) => {
                        rooms.get_mut_by_code(code.room_code).and_then(|room| {
                            let counter = code.room_counter;
                            let id = room.id();
                            let spectating_counter = 0;
                            let karts = karts();
                            let spectating = room.insert(karts)?;
                            Ok(RoomInfo { counter, id, spectating_counter, spectating })
                        })
                    }
                    (ClientRoomState::Main(main), Some(room_info)) => {
                        rooms.get_mut(&room_info.id).and_then(|room| {
                            let spectating_counter = main.spectating_counter;
                            let spectating = if spectating_counter == room_info.spectating_counter {
                                room_info.spectating
                            } else {
                                room.set_spectating(&self.pk, main.spectating != 0)
                            };
                            room.set_options(&self.pk, main.options)?;
                            Ok(RoomInfo { spectating_counter, spectating, ..room_info })
                        })
                    }
                    _ => Err(anyhow!("Unexpected client room state")),
                };
                let room_info = room_info.ok();
                State::Room { identity, room_info }
            }
            _ => anyhow::bail!("Unexpected client state"),
        };
        Ok(())
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
        &self,
        addr: SocketAddr,
        message: &mut [u8],
        player_count: usize,
        rooms: &Rooms,
    ) -> Result<Option<usize>> {
        anyhow::ensure!(addr == self.addr);
        let server_state = match &self.state {
            State::Idle => return Ok(None),
            State::Server { identity } => {
                let protocol_version = PROTOCOL_VERSION;
                let version = version::VERSION.into();
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
                let format_player_counts = (0..3).map(|i| i * 73).collect();
                let pack =
                    ServerStatePack { mode_index, pack_index, player_count, format_player_counts };
                ServerState::Pack(pack)
            }
            State::Room { room_info, .. } => {
                let server_room_state = match room_info {
                    Some(room_info) => {
                        let Ok(room) = rooms.get(&room_info.id) else {
                            return Ok(None);
                        };
                        let karts = room.karts().iter().map(Kart::server_kart).cloned().collect();
                        let spectator_count = room.spectator_count() as u16;
                        let mode_index = room.mode_index();
                        let pack_hash = room.pack_hash().to_vec();
                        let room_counter = room_info.counter;
                        let room_code = room.code();
                        let spectating_counter = room_info.spectating_counter;
                        let spectating = room_info.spectating.into();
                        let options = room.options().clone();
                        let main = ServerRoomStateMain {
                            karts,
                            spectator_count,
                            mode_index,
                            pack_hash,
                            room_counter,
                            room_code,
                            spectating_counter,
                            spectating,
                            options,
                        };
                        ServerRoomState::Main(main)
                    }
                    None => ServerRoomState::Error(()),
                };
                let room = ServerStateRoom { server_room_state };
                ServerState::Room(room)
            }
        };
        let message_len = message.len() - server_state.write(message).unwrap().len();
        Ok(Some(message_len))
    }
}

impl Drop for Client {
    fn drop(&mut self) {
        debug!("<- {}", self.addr);
    }
}

enum State {
    Idle,
    Server { identity: Option<ClientIdentitySpecified> },
    Mode { identity: ClientIdentitySpecified },
    Pack { identity: ClientIdentitySpecified, pack: ClientStatePack },
    Room { identity: ClientIdentitySpecified, room_info: Option<RoomInfo> },
}

struct RoomInfo {
    counter: u32,
    id: u128,
    spectating_counter: u32,
    spectating: bool,
}
