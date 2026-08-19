use std::array;
use std::mem;
use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::{Result, anyhow};
use heapless::{LinearMap, Vec};
use log::{debug, trace};
use rand::Rng;

use crate::config::Config;
use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::formats::version;
use crate::frequency::Frequency;
use crate::kart::Kart;
use crate::pack::Pack;
use crate::player::Player;
use crate::room::Room;
use crate::rooms::{Rooms, Search};
use crate::storage::{PlayerId, Storage};
use crate::update::Update;

pub struct Client {
    expiration: Instant,
    addr: SocketAddr,
    pk: PublicKey,
    state: State,
    client_state: Option<ClientState>,
}

impl Client {
    pub fn new(now: Instant, addr: SocketAddr, pk: PublicKey) -> Self {
        trace!("-> {addr}");
        let expiration = now + Duration::from_mins(2);
        let state = State::Idle;
        let client_state = None;
        Self { expiration, addr, pk, state, client_state }
    }

    pub fn set_addr(&mut self, addr: SocketAddr) {
        debug!("<> {} {addr}", self.addr);
        self.addr = addr;
    }

    fn frequency(&self) -> Frequency {
        match self.state {
            State::Update { .. } => Frequency::FiveHundredHz,
            _ => self
                .state
                .identity()
                .map_or(Frequency::SixtyHz, |identity| identity.frame_rate.into()),
        }
    }

    pub fn player_count(&self) -> usize {
        match self.state {
            State::Server { .. } | State::Update { .. } => 0,
            _ => self.state.identity().map_or(0, |identity| identity.players.len()),
        }
    }

    pub fn room_id(&self) -> Option<u128> {
        self.state.room_info().map(|room_info| room_info.id)
    }

    pub fn update(
        &mut self,
        now: Instant,
        config: &Config,
        frequency: Frequency,
        rooms: &Rooms,
        room_slots: &mut usize,
        storage: &Storage,
        rng: &mut impl Rng,
    ) -> Result<()> {
        anyhow::ensure!(now < self.expiration);
        if frequency != self.frequency() {
            return Ok(());
        }
        let client_state = self.client_state.take();
        let Some(client_state) = client_state else {
            if let State::Update { state: ClientUpdateState::Data(data), .. } = &mut self.state {
                data.indices.pop();
            }
            return Ok(());
        };
        let state = mem::replace(&mut self.state, State::Idle);
        let (identity, room_info) = match state {
            State::Idle => (None, None),
            State::Server { identity } => (identity, None),
            State::Update { identity, .. } => (identity, None),
            State::Mode { identity, .. } => (Some(identity), None),
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
            (ClientState::Update(update), identity, _) => {
                State::Update { identity, state: update.client_update_state }
            }
            (ClientState::Mode(_), Some(identity), _) => {
                let mmrs = (0..identity.players.len())
                    .map(|i| {
                        let id = PlayerId { client_pk: self.pk, index: i as u8 };
                        storage
                            .read_player(&id, |player| player.map(|p| p.mmrs.clone()))
                            .unwrap_or_default()
                    })
                    .collect();
                State::Mode { identity, mmrs }
            }
            (ClientState::Pack(pack), Some(identity), _) => State::Pack { identity, pack },
            (ClientState::Room(room), Some(identity), room_info) => {
                let karts = || {
                    let kart_count = identity.kart_count as usize;
                    let karts: Vec<_, _> = (0..kart_count)
                        .map(|i| {
                            let players = &identity.players;
                            let tandem_count = players.len() - kart_count;

                            let player = |i| {
                                let index = i as u8;
                                let player: &ClientPlayer = &players[i];
                                let name = player.name;
                                let id = PlayerId { client_pk: self.pk, index };
                                storage.read_player(&id, |player| Player {
                                    player: ServerPlayer { index, name },
                                    mmrs: player.map_or(LinearMap::new(), |p| p.mmrs.clone()),
                                    match_count: player.map_or(0, |p| p.race_count),
                                    play_time: player.map_or(Duration::ZERO, |p| p.play_time),
                                })
                            };

                            let players = if i < tandem_count {
                                Vec::from([player(i / 2 + 0), player(i / 2 + 1)])
                            } else {
                                Vec::from([player(i + tandem_count)])
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
                        let mode_index = search.mode_index;
                        let pack =
                            Pack { course_count: search.pack_course_count, hash: search.pack_hash };
                        let format = search.format;
                        let search = Search { mode_index, pack, format };
                        let room = rooms.search(room_slots, frame_rate, &karts, search, rng);
                        room.and_then(|mut room| {
                            let id = room.id();
                            let spectating_counter = 0;
                            let spectating = room.insert(config, karts)?;
                            Ok(RoomInfo { counter, id, spectating_counter, spectating })
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
                        let id = rooms.insert(room_slots, frame_rate, karts, mode_index, pack, rng);
                        id.map(|id| {
                            let counter = new.room_counter;
                            let spectating_counter = 0;
                            let spectating = false;
                            RoomInfo { counter, id, spectating_counter, spectating }
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
                            let spectating = room.insert(config, karts)?;
                            Ok(RoomInfo { counter, id, spectating_counter, spectating })
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
                            room.set_ready(&self.pk);
                            Ok(RoomInfo { spectating_counter, spectating, ..room_info })
                        })
                    }
                    _ => Err(anyhow!("Unexpected client room state")),
                };
                if let Err(e) = &room_info {
                    debug!("{e}");
                }
                let room_info = room_info.ok();
                State::Room { identity, room_info }
            }
            (ClientState::Team(team), Some(identity), room_info) => {
                let room_info = room_info.map_or_else(
                    || Err(anyhow!("Unexpected client team state")),
                    |room_info| {
                        let mut room = rooms.get(&room_info.id)?;
                        room.set_team_state(&self.pk, team.client_team_state)?;
                        Ok(room_info)
                    },
                );
                if let Err(e) = &room_info {
                    debug!("{e}");
                }
                let room_info = room_info.ok();
                State::Team { identity, room_info }
            }
            (ClientState::Poll(poll), Some(identity), room_info) => {
                let room_info = room_info.map_or_else(
                    || Err(anyhow!("Unexpected client poll state")),
                    |room_info| {
                        let mut room = rooms.get(&room_info.id)?;
                        room.set_poll_state(&self.pk, poll.client_poll_state)?;
                        Ok(room_info)
                    },
                );
                if let Err(e) = &room_info {
                    debug!("{e}");
                }
                let room_info = room_info.ok();
                State::Poll { identity, room_info }
            }
            (ClientState::Race(race), Some(identity), room_info) => {
                let frame = race.frame;
                let room_info = room_info.map_or_else(
                    || Err(anyhow!("Unexpected client race state")),
                    |room_info| {
                        let mut room = rooms.get(&room_info.id)?;
                        room.set_race_state(&self.pk, race)?;
                        Ok(room_info)
                    },
                );
                if let Err(e) = &room_info {
                    debug!("{e}");
                }
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
            ClientState::read(message).map_err(|()| anyhow::anyhow!("Invalid client state"))?;
        self.client_state = Some(client_state);
        self.expiration = now + Duration::from_mins(2);
        Ok(())
    }

    pub fn write(
        &self,
        frequency: Frequency,
        addr: SocketAddr,
        message: &mut [u8],
        config: &Config,
        update: Option<&Update>,
        player_count: usize,
        rooms: &Rooms,
    ) -> Result<Option<usize>> {
        anyhow::ensure!(addr == self.addr);
        if frequency != self.frequency() {
            return Ok(None);
        }
        let server_state = match &self.state {
            State::Idle => return Ok(None),
            State::Server { identity } => {
                let server_identity = if identity.is_some() {
                    let specified = ServerIdentitySpecified {
                        motd: config.motd.clone().into_bytes(),
                        player_count: player_count as u16,
                    };
                    ServerIdentity::Specified(specified)
                } else {
                    let unspecified = ServerIdentityUnspecified {};
                    ServerIdentity::Unspecified(unspecified)
                };
                let server = ServerStateServer {
                    update_version: if update.is_some() { UPDATE_VERSION } else { 0 },
                    reserved: 0,
                    protocol_version: PROTOCOL_VERSION,
                    version: version::VERSION.as_bytes().try_into().unwrap(),
                    server_identity,
                };
                ServerState::Server(server)
            }
            State::Update { state, .. } => {
                let Some(update) = update else { return Ok(None) };
                let state = match state {
                    ClientUpdateState::Info(_) => {
                        let info = ServerUpdateStateInfo {
                            size: update.update.len() as u32,
                            changelog: update.changelog.clone(),
                        };
                        ServerUpdateState::Info(info)
                    }
                    ClientUpdateState::Data(data) => {
                        let Some(index) = data.indices.last() else { return Ok(None) };
                        let offset = *index as usize * UPDATE_CHUNK_SIZE;
                        let mut chunk = [0; _];
                        let Some(update) = update.update.get(offset..) else { return Ok(None) };
                        let len = chunk.len().min(update.len());
                        chunk[..len].copy_from_slice(&update[..len]);
                        let data = ServerUpdateStateData { index: *index, chunk };
                        ServerUpdateState::Data(data)
                    }
                };
                let update = ServerStateUpdate { server_update_state: state };
                ServerState::Update(update)
            }
            State::Mode { identity, mmrs } => {
                let mode_player_counts = rooms.mode_player_counts(identity.frame_rate);
                let modes = array::from_fn(|i| {
                    let mmrs = (0..identity.players.len())
                        .map(|j| {
                            mmrs[j]
                                .iter()
                                .find(|(mode_index, _)| **mode_index as usize == i)
                                .map_or(0, |(_, mmr)| *mmr)
                        })
                        .collect();
                    ServerMode { mmrs, player_count: mode_player_counts[i] as u16 }
                });
                let mode = ServerStateMode { modes };
                ServerState::Mode(mode)
            }
            State::Pack { identity, pack, .. } => {
                let ClientStatePack { mode_index, pack_index, pack_course_count, pack_hash } =
                    pack.clone();
                let formats: [_; FORMAT_COUNT] = [
                    RoomOptionFormat::FreeForAll,
                    RoomOptionFormat::TeamsOf2,
                    RoomOptionFormat::TeamsOf4,
                    RoomOptionFormat::Duel,
                ];
                let format_player_counts = formats.map(|format| {
                    let pack = Pack { course_count: pack_course_count, hash: pack_hash };
                    let search = Search { mode_index, pack, format };
                    rooms.search_player_count(identity.frame_rate, &search) as u16
                });
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
                                        .map(|player| player.player.clone())
                                        .collect();
                                    ServerKart {
                                        local,
                                        players,
                                        mmr: kart.mmr(room.mode_index()),
                                        points: kart.points,
                                    }
                                })
                                .collect();
                            let pack = room.pack();
                            ServerRoomStateMain {
                                karts,
                                spectator_count: room.spectator_count() as u16,
                                mode_index: room.mode_index(),
                                pack_course_count: pack.course_count,
                                pack_hash: pack.hash,
                                room_counter: room_info.counter,
                                room_code: room.code().unwrap_or(u64::MAX),
                                spectating_counter: room_info.spectating_counter,
                                spectating: room_info.spectating.into(),
                                options: room.options().clone(),
                                continuing: room.has_room_lock().into(),
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
                        ServerTeamState::Main(main)
                    }
                    None => ServerTeamState::Error(()),
                };
                let team = ServerStateTeam { server_team_state };
                ServerState::Team(team)
            }
            State::Poll { room_info, .. } => {
                let server_poll_state = match room_info {
                    Some(room_info) => {
                        let poll_state = rooms.read(&room_info.id, Room::poll_state);
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
                        let main = rooms
                            .read(&room_info.id, |room| room.race_state(&self.pk, *client_frame));
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
        trace!("<- {}", self.addr);
    }
}

#[derive(Clone, Debug)]
enum State {
    Idle,
    Server {
        identity: Option<ClientIdentitySpecified>,
    },
    Update {
        identity: Option<ClientIdentitySpecified>,
        state: ClientUpdateState,
    },
    Mode {
        identity: ClientIdentitySpecified,
        mmrs: Vec<LinearMap<ModeIndex, u16, MODE_INDEX_COUNT>, MAX_CLIENT_PLAYER_COUNT>,
    },
    Pack {
        identity: ClientIdentitySpecified,
        pack: ClientStatePack,
    },
    Room {
        identity: ClientIdentitySpecified,
        room_info: Option<RoomInfo>,
    },
    Team {
        identity: ClientIdentitySpecified,
        room_info: Option<RoomInfo>,
    },
    Poll {
        identity: ClientIdentitySpecified,
        room_info: Option<RoomInfo>,
    },
    Race {
        identity: ClientIdentitySpecified,
        room_info: Option<RoomInfo>,
        frame: u16,
    },
}

impl State {
    const fn identity(&self) -> Option<&ClientIdentitySpecified> {
        match self {
            Self::Idle => None,
            Self::Server { identity } => identity.as_ref(),
            Self::Update { identity, .. } => identity.as_ref(),
            Self::Mode { identity, .. } => Some(identity),
            Self::Pack { identity, .. } => Some(identity),
            Self::Room { identity, .. } => Some(identity),
            Self::Team { identity, .. } => Some(identity),
            Self::Poll { identity, .. } => Some(identity),
            Self::Race { identity, .. } => Some(identity),
        }
    }

    const fn room_info(&self) -> Option<RoomInfo> {
        match self {
            Self::Room { room_info, .. } => *room_info,
            Self::Team { room_info, .. } => *room_info,
            Self::Poll { room_info, .. } => *room_info,
            Self::Race { room_info, .. } => *room_info,
            _ => None,
        }
    }
}

#[derive(Clone, Copy, Debug)]
struct RoomInfo {
    counter: u32,
    id: u128,
    spectating_counter: u32,
    spectating: bool,
}
