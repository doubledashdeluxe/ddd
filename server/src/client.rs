use std::mem;
use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::{Result, anyhow};
use log::debug;
use rand::Rng;

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::formats::version;
use crate::kart::Kart;
use crate::mmr::Mmr;
use crate::pack::Pack;
use crate::player::Player;
use crate::rooms::{Rooms, Search};

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
            State::Poll { identity, .. } => identity,
            State::Race { identity, .. } => identity,
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
            State::Poll { identity, .. } => identity,
            State::Race { identity, .. } => identity,
            _ => return 0,
        };
        identity.players.len()
    }

    pub fn room_id(&self) -> Option<u128> {
        let room_info = match &self.state {
            State::Room { room_info: Some(room_info), .. } => room_info,
            State::Team { room_info: Some(room_info), .. } => room_info,
            State::Poll { room_info: Some(room_info), .. } => room_info,
            State::Race { room_info: Some(room_info), .. } => room_info,
            _ => return None,
        };
        Some(room_info.id)
    }

    pub fn update(&mut self, now: Instant, rooms: &Rooms, rng: &mut impl Rng) -> Result<()> {
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
            State::Poll { identity, room_info } => (Some(identity), room_info),
            State::Race { identity, room_info, .. } => (Some(identity), room_info),
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
                let mut karts = || {
                    let kart_count = identity.kart_count as usize;
                    let karts: Vec<_> = (0..kart_count)
                        .map(|i| {
                            let players = &identity.players;
                            let tandem_count = players.len() - kart_count;

                            let mut player = |i| {
                                let index = i as u8;
                                let player: &ClientPlayer = &players[i];
                                let name = player.name.clone();
                                let player = ServerPlayer { index, name };
                                let mmr = rng.random_range(..=9999);
                                Player::new(player, mmr)
                            };

                            let players = if i < tandem_count {
                                heapless::Vec::from([player(i / 2 + 0), player(i / 2 + 1)])
                            } else {
                                heapless::Vec::from([player(i + tandem_count)])
                            };
                            Kart::new(self.pk, players)
                        })
                        .collect();
                    karts
                };

                let room_info = match (room.client_room_state, room_info) {
                    (ClientRoomState::Search(search), Some(room_info))
                        if room_info.counter == search.room_counter =>
                    {
                        let id = room_info.id;
                        rooms.read(&id, |_| room_info)
                    }
                    (ClientRoomState::Search(search), _) => {
                        let counter = search.room_counter;
                        let frame_rate = identity.frame_rate;
                        let karts = karts();
                        let is_duel = search.is_duel != 0;
                        let mode_index = search.mode_index;
                        let pack =
                            Pack { course_count: search.pack_course_count, hash: search.pack_hash };
                        let format = search.format;
                        let search = Search { is_duel, mode_index, pack, format };
                        let room = rooms.search(frame_rate, &karts, search, rng);
                        room.and_then(|mut room| {
                            let id = room.id();
                            let spectating_counter = 0;
                            let spectating = room.insert(karts)?;
                            let continuing = room.has_room_lock();
                            Ok(RoomInfo { counter, id, spectating_counter, spectating, continuing })
                        })
                    }
                    (ClientRoomState::New(new), Some(room_info))
                        if room_info.counter == new.room_counter =>
                    {
                        let id = room_info.id;
                        rooms.read(&id, |_| room_info)
                    }
                    (ClientRoomState::New(new), _) => {
                        let frame_rate = identity.frame_rate;
                        let karts = karts();
                        let mode_index = new.mode_index;
                        let pack =
                            Pack { course_count: new.pack_course_count, hash: new.pack_hash };
                        let id = rooms.insert(frame_rate, karts, mode_index, pack, rng);
                        id.map(|id| {
                            let counter = new.room_counter;
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
            (ClientState::Poll(poll), Some(identity), room_info) => {
                let room_info = match room_info {
                    Some(room_info) => rooms.get(&room_info.id).and_then(|mut room| {
                        room.set_poll_state(&self.pk, poll.client_poll_state)?;
                        Ok(room_info)
                    }),
                    None => Err(anyhow!("Unexpected client poll state")),
                };
                let room_info = room_info.ok();
                State::Poll { identity, room_info }
            }
            (ClientState::Race(race), Some(identity), room_info) => {
                let frame = race.frame;
                let room_info = match room_info {
                    Some(room_info) => rooms.get(&room_info.id).and_then(|mut room| {
                        room.set_race(&self.pk, race)?;
                        Ok(room_info)
                    }),
                    None => Err(anyhow!("Unexpected client race state")),
                };
                let room_info = room_info.ok();
                State::Race { identity, room_info, frame }
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
                let version = heapless::Vec::try_from(version::VERSION.as_bytes())?;
                let server_identity = if identity.is_some() {
                    let motd = heapless::Vec::try_from("test motd".as_bytes())?;
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
                let modes = rooms
                    .mode_player_counts(frame_rate)
                    .iter()
                    .enumerate()
                    .map(|(i, player_count)| {
                        let mmrs =
                            (0..identity.players.len()).map(|j| (i * 4 + j) as u16 * 179).collect();
                        ServerMode { mmrs, player_count: *player_count as u16 }
                    })
                    .collect();
                let mode = ServerStateMode { modes };
                ServerState::Mode(mode)
            }
            State::Pack { pack, .. } => {
                let ClientStatePack {
                    is_duel,
                    mode_index,
                    pack_index,
                    pack_course_count,
                    pack_hash,
                } = pack.clone();
                let formats = [
                    RoomOptionFormat::FreeForAll,
                    RoomOptionFormat::TeamsOf2,
                    RoomOptionFormat::TeamsOf4,
                ];
                let format_player_counts: heapless::Vec<_, _> = formats
                    .into_iter()
                    .map(|format| {
                        let pack =
                            Pack { course_count: pack_course_count, hash: pack_hash.clone() };
                        let search = Search { is_duel: is_duel != 0, mode_index, pack, format };
                        rooms.search_player_count(frame_rate, &search) as u16
                    })
                    .collect();
                let player_count = format_player_counts.iter().sum();
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
                                .map(|kart| {
                                    let local = (kart.client_pk() == &self.pk).into();
                                    let players = kart
                                        .players()
                                        .iter()
                                        .map(&Player::player)
                                        .cloned()
                                        .collect();
                                    let mmr = kart.mmr();
                                    ServerKart { local, players, mmr }
                                })
                                .collect();
                            let pack = room.pack();
                            ServerRoomStateMain {
                                karts,
                                spectator_count: room.spectator_count() as u16,
                                mode_index: room.mode_index(),
                                pack_course_count: pack.course_count,
                                pack_hash: pack.hash.clone(),
                                room_counter: room_info.counter,
                                room_code: room.code().unwrap_or(u64::MAX),
                                spectating_counter: room_info.spectating_counter,
                                spectating: room_info.spectating.into(),
                                options: room.options().clone(),
                                continuing: room_info.continuing.into(),
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
            State::Poll { room_info, .. } => {
                let server_poll_state = match room_info {
                    Some(room_info) => {
                        let poll_state = rooms.read(&room_info.id, |room| room.poll_state());
                        let Ok(Some(poll_state)) = poll_state else {
                            return Ok(None);
                        };
                        poll_state
                    }
                    None => ServerPollState::Error(()),
                };
                let poll = ServerStatePoll { server_poll_state };
                ServerState::Poll(poll)
            }
            State::Race { room_info, frame: client_frame, .. } => {
                let server_race_state = match room_info {
                    Some(room_info) => {
                        let main = rooms.read(&room_info.id, |room| room.race_state(*client_frame));
                        let Ok(Some(main)) = main else {
                            return Ok(None);
                        };
                        ServerRaceState::Main(main)
                    }
                    None => ServerRaceState::Error(()),
                };
                let race = ServerStateRace { server_race_state };
                ServerState::Race(race)
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
    Poll { identity: ClientIdentitySpecified, room_info: Option<RoomInfo> },
    Race { identity: ClientIdentitySpecified, room_info: Option<RoomInfo>, frame: u16 },
}

struct RoomInfo {
    counter: u32,
    id: u128,
    spectating_counter: u32,
    spectating: bool,
    continuing: bool,
}
