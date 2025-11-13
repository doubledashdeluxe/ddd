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

    fn frame_rate(&self) -> Option<FrameRate> {
        let identity = match &self.state {
            State::Server { identity: Some(identity) } => identity,
            State::Mode { identity } => identity,
            State::Pack { identity, .. } => identity,
            State::Room { identity, .. } => identity,
            State::Team { identity, .. } => identity,
            _ => return None,
        };
        Some(identity.frame_rate)
    }

    pub fn player_count(&self) -> usize {
        let identity = match &self.state {
            State::Mode { identity } => identity,
            State::Pack { identity, .. } => identity,
            State::Room { identity, .. } => identity,
            State::Team { identity, .. } => identity,
            _ => return 0,
        };
        identity.players.len()
    }

    pub fn room_id(&self) -> Option<u128> {
        let room_info = match &self.state {
            State::Room { room_info: Some(room_info), .. } => room_info,
            State::Team { room_info: Some(room_info), .. } => room_info,
            _ => return None,
        };
        Some(room_info.id)
    }

    pub fn update(&mut self, now: Instant, rooms: &Rooms) -> Result<()> {
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
            State::Team { identity, room_info } => (Some(identity), room_info),
        };
        self.state = match (client_state, identity, room_info) {
            (ClientState::Server(server), _, _) => {
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
            (ClientState::Mode(_), Some(identity), _) => State::Mode { identity },
            (ClientState::Pack(pack), Some(identity), _) => State::Pack { identity, pack },
            (ClientState::Room(room), Some(identity), room_info) => {
                let karts = || {
                    let kart_count = identity.kart_count as usize;
                    let karts: Vec<_> = (0..kart_count)
                        .map(|i| {
                            let players = &identity.players;
                            let tandem_count = players.len() - kart_count;

                            let player = |i| {
                                let index = i as u8;
                                let player: &ClientPlayer = &players[i];
                                let name = player.name.clone();
                                ServerPlayer { index, name }
                            };

                            let players = if i < tandem_count {
                                vec![player(i / 2 + 0), player(i / 2 + 1)]
                            } else {
                                vec![player(i + tandem_count)]
                            };
                            Kart::new(self.pk, players)
                        })
                        .collect();
                    karts
                };

                let room_info = match (room.client_room_state, room_info) {
                    (ClientRoomState::New(new), Some(room_info))
                        if room_info.counter == new.room_counter =>
                    {
                        let id = room_info.id;
                        rooms.read(&id, |_| room_info)
                    }
                    (ClientRoomState::New(new), _) => {
                        let counter = new.room_counter;
                        let frame_rate = identity.frame_rate;
                        let karts = karts();
                        let mode_index = new.mode_index;
                        let pack_hash = new.pack_hash;
                        let room = rooms.insert(frame_rate, karts, mode_index, pack_hash);
                        room.map(|id| {
                            let spectating_counter = 0;
                            let spectating = false;
                            let continuing = false;
                            RoomInfo { counter, id, spectating_counter, spectating, continuing }
                        })
                    }
                    (ClientRoomState::Code(code), Some(room_info))
                        if code.room_counter == room_info.counter =>
                    {
                        let id = room_info.id;
                        rooms.read(&id, |_| room_info)
                    }
                    (ClientRoomState::Code(code), _) => {
                        let frame_rate = identity.frame_rate;
                        let room = rooms.get_by_frame_rate_and_code(frame_rate, code.room_code);
                        room.and_then(|mut room| {
                            let counter = code.room_counter;
                            let id = room.id();
                            let spectating_counter = 0;
                            let karts = karts();
                            let spectating = room.insert(karts)?;
                            let continuing = room.has_room_lock();
                            Ok(RoomInfo { counter, id, spectating_counter, spectating, continuing })
                        })
                    }
                    (ClientRoomState::Main(main), Some(room_info)) => {
                        rooms.get(&room_info.id).and_then(|mut room| {
                            let spectating_counter = main.spectating_counter;
                            let spectating = if spectating_counter == room_info.spectating_counter {
                                room_info.spectating
                            } else {
                                room.set_spectating(&self.pk, main.spectating != 0)
                            };
                            room.set_options(&self.pk, main.options)?;
                            room.set_continuing(&self.pk, main.continuing != 0)?;
                            let continuing = room.has_room_lock();
                            Ok(RoomInfo { spectating_counter, spectating, continuing, ..room_info })
                        })
                    }
                    _ => Err(anyhow!("Unexpected client room state")),
                };
                let room_info = room_info.ok();
                State::Room { identity, room_info }
            }
            (ClientState::Team(team), Some(identity), room_info) => {
                let room_info = match room_info {
                    Some(room_info) => rooms.get(&room_info.id).and_then(|mut room| {
                        room.set_team_state(&self.pk, team.client_team_state)?;
                        Ok(room_info)
                    }),
                    None => Err(anyhow!("Unexpected client team state")),
                };
                let room_info = room_info.ok();
                State::Team { identity, room_info }
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
        frame_rate: FrameRate,
        addr: SocketAddr,
        message: &mut [u8],
        player_count: usize,
        rooms: &Rooms,
    ) -> Result<Option<usize>> {
        anyhow::ensure!(addr == self.addr);
        if frame_rate != self.frame_rate().unwrap_or(FrameRate::SixtyHz) {
            return Ok(None);
        }
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
                        let main = rooms.read(&room_info.id, |room| {
                            let karts = room
                                .karts()
                                .iter()
                                .enumerate()
                                .map(|(i, kart)| {
                                    let local = (kart.client_pk() == &self.pk).into();
                                    let players = kart.players().to_vec();
                                    let mmr = i as u16 * 1023;
                                    ServerKart { local, players, mmr }
                                })
                                .collect();
                            let spectator_count = room.spectator_count() as u16;
                            let mode_index = room.mode_index();
                            let pack_hash = room.pack_hash().to_vec();
                            let room_counter = room_info.counter;
                            let room_code = room.code();
                            let spectating_counter = room_info.spectating_counter;
                            let spectating = room_info.spectating.into();
                            let options = room.options().clone();
                            let continuing = room_info.continuing.into();
                            ServerRoomStateMain {
                                karts,
                                spectator_count,
                                mode_index,
                                pack_hash,
                                room_counter,
                                room_code,
                                spectating_counter,
                                spectating,
                                options,
                                continuing,
                            }
                        });
                        let Ok(main) = main else {
                            return Ok(None);
                        };
                        ServerRoomState::Main(main)
                    }
                    None => ServerRoomState::Error(()),
                };
                let room = ServerStateRoom { server_room_state };
                ServerState::Room(room)
            }
            State::Team { room_info, .. } => {
                let server_team_state = match room_info {
                    Some(room_info) => {
                        let main = rooms.read(&room_info.id, |room| room.team_state().cloned());
                        let Ok(Some(main)) = main else {
                            return Ok(None);
                        };
                        ServerTeamState::Main(main.clone())
                    }
                    None => ServerTeamState::Error(()),
                };
                let team = ServerStateTeam { server_team_state };
                ServerState::Team(team)
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
    Team { identity: ClientIdentitySpecified, room_info: Option<RoomInfo> },
}

struct RoomInfo {
    counter: u32,
    id: u128,
    spectating_counter: u32,
    spectating: bool,
    continuing: bool,
}
