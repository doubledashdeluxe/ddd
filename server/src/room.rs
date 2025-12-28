use std::array;
use std::collections::hash_map::{Entry, HashMap};
use std::ops::BitOr;
use std::time::{Duration, Instant};

use anyhow::Result;
use log::debug;
use rand::seq::SliceRandom;
use rand::{Rng, SeedableRng};

use crate::crypto::{ChaCha20Rng, PublicKey};
use crate::formats::online::*;
use crate::kart::Kart;
use crate::mmr::Mmr;
use crate::pack::Pack;

pub struct Room {
    host_pk: Option<PublicKey>,
    karts: Vec<Kart>,
    spectating_karts: HashMap<PublicKey, Vec<Kart>>,
    spectator_count: usize,
    max_kart_count: usize,
    max_client_kart_count: usize,
    mode_index: ModeIndex,
    pack: Pack,
    id: u128,
    code_pair: Option<CodePair>,
    options: ServerRoomOptions,
    state: State,
    rng: ChaCha20Rng,
}

impl Room {
    pub fn new_worldwide(
        mode_index: ModeIndex,
        pack: Pack,
        id: u128,
        format: RoomOptionFormat,
        rng: &mut impl Rng,
    ) -> Self {
        let config = Config {
            host_karts: vec![],
            max_kart_count: MAX_ROOM_KART_COUNT as usize,
            max_client_kart_count: MAX_CLIENT_KART_COUNT as usize,
            mode_index,
            pack,
            id,
            code_pair: None,
            format,
            match_count: 0,
            rng,
        };
        Self::new(config)
    }

    pub fn new_duel(pack: Pack, id: u128, rng: &mut impl Rng) -> Self {
        let config = Config {
            host_karts: vec![],
            max_kart_count: 2,
            max_client_kart_count: 1,
            mode_index: ModeIndex::Versus,
            pack,
            id,
            code_pair: None,
            format: RoomOptionFormat::FreeForAll,
            match_count: 1,
            rng,
        };
        Self::new(config)
    }

    pub fn new_personal(
        host_karts: Vec<Kart>,
        mode_index: ModeIndex,
        pack: Pack,
        id: u128,
        code_pair: CodePair,
        rng: &mut impl Rng,
    ) -> Self {
        let config = Config {
            host_karts,
            max_kart_count: MAX_ROOM_KART_COUNT as usize,
            max_client_kart_count: MAX_CLIENT_KART_COUNT as usize,
            mode_index,
            pack,
            id,
            code_pair: Some(code_pair),
            format: RoomOptionFormat::FreeForAll,
            match_count: DEFAULT_MATCH_COUNT,
            rng,
        };
        Self::new(config)
    }

    fn new(config: Config<'_, impl Rng>) -> Room {
        debug!("-> {}", config.id);
        let is_race = match config.mode_index {
            ModeIndex::Versus => true,
            ModeIndex::Balloon => false,
            ModeIndex::Escape => false,
            ModeIndex::Bomb => false,
            ModeIndex::TimeAttack => true,
        };
        let options = if is_race {
            let options = RoomOptionsRace {
                race: (),
                code_type: RoomOptionCodeType::Long,
                format: config.format,
                engine_size: RoomOptionEngineSize::Large,
                item_mode: RoomOptionItemMode::Recommended,
                lap_count: 0,
                match_count: config.match_count,
                course_selection: RoomOptionCourseSelection::Poll,
                entry_index: 0,
            };
            ServerRoomOptions::RaceOptions(options)
        } else {
            let options = RoomOptionsBattle {
                battle: (),
                code_type: RoomOptionCodeType::Long,
                format: config.format,
                item_mode: RoomOptionItemMode::Recommended,
                match_count: config.match_count,
                course_selection: RoomOptionCourseSelection::Poll,
                entry_index: 0,
            };
            ServerRoomOptions::BattleOptions(options)
        };
        Room {
            host_pk: config.host_karts.first().map(Kart::client_pk).copied(),
            karts: config.host_karts,
            spectating_karts: HashMap::new(),
            spectator_count: 0,
            max_kart_count: config.max_kart_count,
            max_client_kart_count: config.max_client_kart_count,
            mode_index: config.mode_index,
            pack: config.pack,
            id: config.id,
            code_pair: config.code_pair,
            options,
            state: State::new_room(),
            rng: ChaCha20Rng::from_rng(config.rng),
        }
    }

    fn is_host(&self, client_pk: &PublicKey) -> bool {
        Some(client_pk) == self.host_pk.as_ref()
    }

    fn is_guest(&self, client_pk: &PublicKey) -> bool {
        !self.is_host(client_pk)
    }

    pub fn karts(&self) -> &[Kart] {
        &self.karts
    }

    pub fn player_count(&self) -> usize {
        self.karts.iter().map(|kart| kart.players().len()).sum()
    }

    pub fn spectating_kart_count(&self) -> usize {
        self.spectating_karts.values().map(|karts| karts.len()).sum()
    }

    pub fn spectator_count(&self) -> usize {
        self.spectator_count
    }

    pub fn mode_index(&self) -> ModeIndex {
        self.mode_index
    }

    pub fn pack(&self) -> &Pack {
        &self.pack
    }

    pub fn id(&self) -> u128 {
        self.id
    }

    pub fn options(&self) -> &ServerRoomOptions {
        &self.options
    }

    pub fn code(&self) -> Option<u64> {
        match (&self.code_pair, self.code_type()) {
            (Some(code_pair), RoomOptionCodeType::Long) => Some(code_pair.long),
            (Some(code_pair), RoomOptionCodeType::Short) => Some(code_pair.short),
            (None, _) => None,
        }
    }

    fn code_type(&self) -> RoomOptionCodeType {
        match &self.options {
            ServerRoomOptions::RaceOptions(options) => options.code_type,
            ServerRoomOptions::BattleOptions(options) => options.code_type,
        }
    }

    fn is_ffa(&self) -> bool {
        matches!(self.format(), RoomOptionFormat::FreeForAll)
    }

    fn team_count(&self) -> u8 {
        let max_team_size = match self.format() {
            RoomOptionFormat::FreeForAll => 1,
            RoomOptionFormat::TeamsOf2 => 2,
            RoomOptionFormat::TeamsOf4 => 4,
        };
        self.karts.len().div_ceil(max_team_size).max(2) as u8
    }

    fn balance_teams(&mut self, teams: &mut [u8]) {
        let mut team_sizes = [0; MAX_TEAM_COUNT as usize];
        for team in teams.iter() {
            team_sizes[*team as usize] += 1;
        }

        let kart_count = teams.len();
        let mut karts: [_; MAX_ROOM_KART_COUNT as usize] = array::from_fn(|i| i);
        karts[..kart_count].shuffle(&mut self.rng);

        let team_count = self.team_count() as usize;
        let mut other_teams: [_; MAX_TEAM_COUNT as usize] = array::from_fn(|i| i);
        other_teams[..team_count].shuffle(&mut self.rng);

        let max_team_size = kart_count.div_ceil(team_count);
        for kart in karts[..kart_count].iter() {
            let team = &mut teams[*kart];
            let team_size = team_sizes[*team as usize];
            if team_size <= max_team_size {
                continue;
            }

            for other_team in other_teams[..team_count].iter() {
                let other_team_size = team_sizes[*other_team];
                if other_team_size < max_team_size {
                    team_sizes[*team as usize] -= 1;
                    *team = *other_team as u8;
                    team_sizes[*other_team] += 1;
                    break;
                }
            }
        }
    }

    fn format(&self) -> RoomOptionFormat {
        match &self.options {
            ServerRoomOptions::RaceOptions(options) => options.format,
            ServerRoomOptions::BattleOptions(options) => options.format,
        }
    }

    fn course_selection(&self) -> RoomOptionCourseSelection {
        match &self.options {
            ServerRoomOptions::RaceOptions(options) => options.course_selection,
            ServerRoomOptions::BattleOptions(options) => options.course_selection,
        }
    }

    pub fn team_state(&self) -> Option<&ServerTeamStateMain> {
        match &self.state {
            State::Room => None,
            State::Team { state, .. } => Some(state),
            State::Poll { team_state, .. } => team_state.as_ref(),
            State::Race { team_state, .. } => team_state.as_ref(),
        }
    }

    pub fn poll_state(&self) -> Option<ServerPollState> {
        match &self.state {
            State::Room => None,
            State::Team { .. } => None,
            State::Poll { state, .. } => Some(ServerPollState::Pending(state.clone())),
            State::Race { poll_state, .. } => Some(ServerPollState::Ready(poll_state.clone())),
        }
    }

    pub fn race_state(&self, frame: u16) -> Option<ServerRaceStateMain> {
        match &self.state {
            State::Room => None,
            State::Team { .. } => None,
            State::Poll { .. } => None,
            State::Race { states, .. } => {
                let mut state = states.get(frame as usize)?.clone();
                state.frame = states.len() as u16 - 1;
                Some(state)
            }
        }
    }

    pub fn has_room_lock(&self) -> bool {
        match self.state {
            State::Room => false,
            State::Team { .. } => true,
            State::Poll { .. } => true,
            State::Race { .. } => true,
        }
    }

    pub fn insert(&mut self, guest_karts: Vec<Kart>) -> Result<bool> {
        anyhow::ensure!(guest_karts.len() <= self.max_client_kart_count);
        if !self.has_room_lock() && self.karts.len() + guest_karts.len() <= self.max_kart_count {
            self.karts.extend(guest_karts);
            Ok(false)
        } else {
            anyhow::ensure!(self.spectating_karts.len() < 1000);
            let guest_pk = *guest_karts[0].client_pk();
            self.spectating_karts.insert(guest_pk, guest_karts);
            Ok(true)
        }
    }

    pub fn set_spectating(&mut self, client_pk: &PublicKey, spectating: bool) -> bool {
        if self.host_pk.is_none() || self.has_room_lock() {
            return self.spectating_karts.contains_key(client_pk);
        }

        if spectating {
            let karts = self.karts.extract_if(.., |kart| kart.client_pk() == client_pk).collect();
            self.spectating_karts.insert(*client_pk, karts);
        } else if let Entry::Occupied(o) = self.spectating_karts.entry(*client_pk) {
            if o.get().len() + self.karts.len() > self.max_kart_count {
                return true;
            }

            let karts = o.remove();
            if self.is_host(client_pk) {
                self.karts.splice(0..0, karts);
            } else {
                self.karts.extend(karts);
            }
        }
        spectating
    }

    pub fn set_options(&mut self, client_pk: &PublicKey, options: ClientRoomOptions) -> Result<()> {
        match (options, &self.options) {
            (ClientRoomOptions::RaceOptions(client), ServerRoomOptions::RaceOptions(server)) => {
                anyhow::ensure!(self.is_host(client_pk));
                if self.has_room_lock() {
                    anyhow::ensure!(client == *server);
                } else {
                    anyhow::ensure!(client.lap_count <= MAX_LAP_COUNT);
                    anyhow::ensure!(client.match_count >= MIN_MATCH_COUNT);
                    anyhow::ensure!(client.match_count <= MAX_MATCH_COUNT);
                    self.options = ServerRoomOptions::RaceOptions(client);
                }
            }
            (
                ClientRoomOptions::BattleOptions(client),
                ServerRoomOptions::BattleOptions(server),
            ) => {
                anyhow::ensure!(self.is_host(client_pk));
                if self.has_room_lock() {
                    anyhow::ensure!(client == *server);
                } else {
                    anyhow::ensure!(client.match_count >= MIN_MATCH_COUNT);
                    anyhow::ensure!(client.match_count <= MAX_MATCH_COUNT);
                    self.options = ServerRoomOptions::BattleOptions(client);
                }
            }
            (ClientRoomOptions::None(()), _) => {
                anyhow::ensure!(self.is_guest(client_pk))
            }
            _ => anyhow::bail!("Invalid options"),
        }
        Ok(())
    }

    pub fn set_continuing(&mut self, client_pk: &PublicKey, continuing: bool) -> Result<()> {
        if self.is_host(client_pk) {
            if self.has_room_lock() {
                anyhow::ensure!(continuing);
            } else if continuing {
                if self.is_ffa() {
                    self.state = State::new_poll(None);
                } else {
                    let deadline = Instant::now() + Duration::from_secs(35);
                    self.state = State::new_team(self.karts.len(), deadline);
                }
            }
        } else {
            anyhow::ensure!(!continuing);
        }
        Ok(())
    }

    pub fn set_team_state(&mut self, client_pk: &PublicKey, state: ClientTeamState) -> Result<()> {
        match state {
            ClientTeamState::Host(host) => self.set_team_state_host(client_pk, host),
            ClientTeamState::Guest(_) => self.set_team_state_guest(client_pk),
        }
    }

    fn set_team_state_host(
        &mut self,
        client_pk: &PublicKey,
        state: ClientTeamStateHost,
    ) -> Result<()> {
        anyhow::ensure!(self.is_host(client_pk));
        let ClientTeamStateHost { teams, entry_index, continuing } = state;
        anyhow::ensure!(teams.len() == self.karts.len());
        let team_count = self.team_count();
        anyhow::ensure!(teams.iter().all(|team| *team < team_count));
        match &mut self.state {
            State::Team { state, .. } => {
                state.teams = teams;
                state.entry_index = entry_index;
                if continuing != 0 {
                    let mut state = state.clone();
                    self.balance_teams(&mut state.teams);
                    state.continuing = true as u8;
                    self.state = State::new_poll(Some(state));
                }
            }
            State::Poll { team_state: Some(_), .. } => (),
            State::Race { team_state: Some(_), .. } => (),
            _ => anyhow::bail!("Invalid room state"),
        }
        Ok(())
    }

    fn set_team_state_guest(&self, client_pk: &PublicKey) -> Result<()> {
        anyhow::ensure!(self.team_state().is_some());
        anyhow::ensure!(self.is_guest(client_pk));
        Ok(())
    }

    pub fn set_poll_state(&mut self, client_pk: &PublicKey, state: ClientPollState) -> Result<()> {
        match state {
            ClientPollState::Pending(()) => self.set_poll_state_pending(),
            ClientPollState::Ready(ready) => self.set_poll_state_ready(client_pk, ready),
        }
    }

    fn set_poll_state_pending(&self) -> Result<()> {
        anyhow::ensure!(self.poll_state().is_some());
        Ok(())
    }

    fn set_poll_state_ready(
        &mut self,
        client_pk: &PublicKey,
        state: ClientPollStateReady,
    ) -> Result<()> {
        let ClientPollStateReady { karts: client_karts, course_index } = state;
        let kart_count = self.karts.iter().filter(|kart| kart.client_pk() == client_pk).count();
        anyhow::ensure!(client_karts.len() == kart_count);
        for client_kart in &client_karts {
            let ClientPollKart { character_ids, kart_id } = client_kart;
            anyhow::ensure!(character_ids[0] != character_ids[1]);
            anyhow::ensure!(kart_id.compatible(character_ids));
        }
        let course_index = match course_index {
            ClientCourseIndex::Unspecified(()) => None,
            ClientCourseIndex::Specified(course_index) => {
                anyhow::ensure!(course_index < self.pack.course_count);
                Some(course_index)
            }
        };
        let course_selection = self.course_selection();
        let is_host = self.is_host(client_pk);
        match course_selection {
            RoomOptionCourseSelection::Poll => {
                let has_spectating_kart = self.spectating_karts.contains_key(client_pk);
                anyhow::ensure!(course_index.is_some() == !has_spectating_kart);
            }
            RoomOptionCourseSelection::Host => {
                anyhow::ensure!(course_index.is_some() == is_host);
            }
            RoomOptionCourseSelection::Random => {
                anyhow::ensure!(course_index.is_none());
            }
        }
        match &mut self.state {
            State::Poll { team_state, state, karts: server_karts, host_course_index, .. } => {
                let kart_indices = self
                    .karts
                    .iter()
                    .enumerate()
                    .filter(|(_, kart)| kart.client_pk() == client_pk)
                    .map(|(i, _)| i as u8);
                let is_host_selection = course_selection == RoomOptionCourseSelection::Host;
                for (client_kart, kart_index) in client_karts.into_iter().zip(kart_indices) {
                    let Entry::Vacant(v) = server_karts.entry(kart_index) else {
                        continue;
                    };
                    let course_index = match course_index {
                        Some(course_index) if !is_host_selection => course_index,
                        _ => self.rng.random_range(..self.pack.course_count),
                    };
                    let server_kart = ServerPollKart {
                        kart_index,
                        character_ids: client_kart.character_ids,
                        kart_id: client_kart.kart_id,
                        course_index,
                    };
                    state.kart_indices.push(kart_index);
                    v.insert(server_kart);
                }
                if is_host_selection && is_host {
                    *host_course_index = course_index;
                }
                if server_karts.len() == self.karts.len()
                    && (!is_host_selection || host_course_index.is_some())
                {
                    self.state = State::new_race(
                        team_state.clone(),
                        state,
                        server_karts,
                        *host_course_index,
                        &mut self.rng,
                    );
                }
            }
            State::Race { .. } => (),
            _ => anyhow::bail!("Invalid room state"),
        }
        Ok(())
    }

    pub fn set_race(&mut self, client_pk: &PublicKey, race: ClientStateRace) -> Result<()> {
        let ClientStateRace { frame: client_frame, karts: client_karts } = race;
        let kart_count = self.karts.iter().filter(|kart| kart.client_pk() == client_pk).count();
        anyhow::ensure!(client_karts.len() == kart_count);
        match &mut self.state {
            State::Race { karts, .. } => {
                let kart_indices = self
                    .karts
                    .iter()
                    .enumerate()
                    .filter(|(_, kart)| kart.client_pk() == client_pk)
                    .map(|(i, _)| i as u8);
                for (client_kart, kart_index) in client_karts.into_iter().zip(kart_indices) {
                    let kart = ServerRaceKart {
                        kart_frame: client_frame,
                        pos_x: client_kart.pos_x,
                        pos_y: client_kart.pos_y,
                        pos_z: client_kart.pos_z,
                        angle: client_kart.angle,
                        vel_x: client_kart.vel_x,
                        vel_y: client_kart.vel_y,
                        vel_z: client_kart.vel_z,
                    };
                    karts[kart_index as usize] = Some(kart);
                }
            }
            _ => anyhow::bail!("Invalid room state"),
        }
        Ok(())
    }

    pub fn update(&mut self, client_room_ids: &HashMap<PublicKey, Option<u128>>) -> Result<()> {
        let present = |client_pk: &PublicKey| {
            client_room_ids.get(client_pk).is_some_and(|room_id| *room_id == Some(self.id))
        };

        if !self.has_room_lock() {
            self.karts.retain(|kart| present(kart.client_pk()))
        }
        self.spectating_karts.retain(|client_pk, _| present(client_pk));

        if let Some(host_pk) = &self.host_pk
            && !self.has_room_lock()
        {
            let has_host_kart = || self.karts.first().map(Kart::client_pk) == Some(host_pk);
            let has_host_spectating_kart = || self.spectating_karts.contains_key(host_pk);
            anyhow::ensure!(has_host_kart() || has_host_spectating_kart());
        } else {
            anyhow::ensure!(self.karts.iter().any(|kart| present(kart.client_pk())));
        }

        self.spectator_count = self
            .spectating_karts
            .values()
            .flat_map(|karts| karts.iter().map(|kart| kart.players().len()))
            .sum();

        let course_selection = self.course_selection();
        match &mut self.state {
            State::Room if self.host_pk.is_none() => {
                self.spectating_karts.retain(|_, karts| {
                    let retain = self.karts.len() + karts.len() > self.max_kart_count;
                    if !retain {
                        self.karts.append(karts);
                    }
                    retain
                });
                if self.karts.windows(2).any(|karts| karts[0].client_pk() != karts[1].client_pk()) {
                    if self.is_ffa() {
                        self.state = State::new_poll(None);
                    } else {
                        let deadline = Instant::now();
                        self.state = State::new_team(self.karts.len(), deadline);
                    }
                }
            }
            State::Team { state, deadline } if Instant::now() >= *deadline => {
                let mut state = state.clone();
                self.balance_teams(&mut state.teams);
                state.continuing = true as u8;
                self.state = State::new_poll(Some(state));
            }
            State::Poll { team_state, state, karts, host_course_index, deadline }
                if Instant::now() >= *deadline =>
            {
                for (kart_index, kart) in self.karts.iter().enumerate() {
                    let kart_index = kart_index as u8;
                    let is_host = Some(kart.client_pk()) == self.host_pk.as_ref();
                    let Entry::Vacant(v) = karts.entry(kart_index) else {
                        continue;
                    };
                    let character_ids: Vec<_> = (0..2).map(|_| self.rng.random()).collect();
                    let kart_id = loop {
                        let kart_id: KartId = self.rng.random();
                        if kart_id.compatible(&character_ids) {
                            break kart_id;
                        }
                    };
                    let course_index = self.rng.random_range(..self.pack.course_count);
                    let kart = ServerPollKart { kart_index, character_ids, kart_id, course_index };
                    state.kart_indices.push(kart_index);
                    v.insert(kart);
                    if course_selection == RoomOptionCourseSelection::Host && is_host {
                        host_course_index.get_or_insert(course_index);
                    }
                }
                if course_selection == RoomOptionCourseSelection::Host {
                    host_course_index
                        .get_or_insert_with(|| self.rng.random_range(..self.pack.course_count));
                }
                self.state = State::new_race(
                    team_state.clone(),
                    state,
                    karts,
                    *host_course_index,
                    &mut self.rng,
                );
            }
            State::Race { karts, states, .. } => {
                let kart_flags = karts
                    .iter()
                    .enumerate()
                    .map(|(i, kart)| (kart.is_some() as u8) << i)
                    .fold(0, BitOr::bitor);
                let karts = karts.iter().filter_map(|kart| kart.clone()).collect();
                let state = ServerRaceStateMain {
                    frame: states.len() as u16,
                    client_frame: states.len() as u16,
                    kart_flags,
                    karts,
                };
                states.push(state);
            }
            _ => (),
        }

        Ok(())
    }
}

impl Drop for Room {
    fn drop(&mut self) {
        debug!("<- {}", self.id);
    }
}

impl Mmr for Room {
    fn mmr(&self) -> u16 {
        self.karts.mmr()
    }
}

#[derive(Clone, Copy, Debug)]
pub struct CodePair {
    pub long: u64,
    pub short: u64,
}

struct Config<'a, R: Rng> {
    host_karts: Vec<Kart>,
    max_kart_count: usize,
    max_client_kart_count: usize,
    mode_index: ModeIndex,
    pack: Pack,
    id: u128,
    code_pair: Option<CodePair>,
    format: RoomOptionFormat,
    match_count: u8,
    rng: &'a mut R,
}

enum State {
    Room,
    Team {
        state: ServerTeamStateMain,
        deadline: Instant,
    },
    Poll {
        team_state: Option<ServerTeamStateMain>,
        state: ServerPollStatePending,
        karts: HashMap<u8, ServerPollKart>,
        host_course_index: Option<u8>,
        deadline: Instant,
    },
    Race {
        team_state: Option<ServerTeamStateMain>,
        poll_state: ServerPollStateReady,
        karts: Vec<Option<ServerRaceKart>>,
        states: Vec<ServerRaceStateMain>,
    },
}

impl State {
    fn new_room() -> Self {
        Self::Room
    }

    fn new_team(kart_count: usize, deadline: Instant) -> Self {
        let state = ServerTeamStateMain {
            teams: vec![0; kart_count],
            entry_index: 0,
            continuing: false as u8,
        };
        Self::Team { state, deadline }
    }

    fn new_poll(team_state: Option<ServerTeamStateMain>) -> Self {
        Self::Poll {
            team_state,
            state: ServerPollStatePending { kart_indices: vec![] },
            karts: HashMap::new(),
            host_course_index: None,
            deadline: Instant::now() + Duration::from_secs(35),
        }
    }

    fn new_race(
        team_state: Option<ServerTeamStateMain>,
        state: &ServerPollStatePending,
        karts: &mut HashMap<u8, ServerPollKart>,
        host_course_index: Option<u8>,
        rng: &mut impl Rng,
    ) -> Self {
        let mut karts: Vec<_> =
            state.kart_indices.iter().map(|kart_index| karts.remove(kart_index).unwrap()).collect();
        let kart_count = karts.len();
        let selected_kart_index = rng.random_range(..kart_count as u8);
        if let Some(host_course_index) = host_course_index {
            karts[selected_kart_index as usize].course_index = host_course_index;
        }
        Self::Race {
            team_state,
            poll_state: ServerPollStateReady { karts, selected_kart_index },
            karts: vec![None; kart_count],
            states: vec![],
        }
    }
}
