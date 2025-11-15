use std::array;
use std::collections::hash_map::{Entry, HashMap};
use std::time::{Duration, Instant};

use anyhow::Result;
use log::debug;
use rand::seq::SliceRandom;
use rand::{Rng, SeedableRng};

use crate::crypto::{ChaCha20Rng, PublicKey};
use crate::formats::online::*;
use crate::kart::Kart;

pub struct Room {
    host_pk: PublicKey,
    karts: Vec<Kart>,
    spectating_karts: HashMap<PublicKey, Vec<Kart>>,
    spectator_count: usize,
    mode_index: ModeIndex,
    pack_hash: Vec<u8>,
    id: u128,
    long_code: u64,
    short_code: u64,
    options: ServerRoomOptions,
    state: State,
    rng: ChaCha20Rng,
}

impl Room {
    pub fn new(
        host_karts: Vec<Kart>,
        mode_index: ModeIndex,
        pack_hash: Vec<u8>,
        id: u128,
        long_code: u64,
        short_code: u64,
        rng: &mut impl Rng,
    ) -> Room {
        debug!("-> {id}");
        let karts = host_karts;
        let spectating_karts = HashMap::new();
        let spectator_count = 0;
        let host_pk = *karts[0].client_pk();
        let is_race = match mode_index {
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
                format: RoomOptionFormat::FreeForAll,
                engine_size: RoomOptionEngineSize::Large,
                item_mode: RoomOptionItemMode::Recommended,
                lap_count: 0,
                match_count: DEFAULT_MATCH_COUNT,
                course_selection: RoomOptionCourseSelection::Poll,
                entry_index: 0,
            };
            ServerRoomOptions::Race(options)
        } else {
            let options = RoomOptionsBattle {
                battle: (),
                code_type: RoomOptionCodeType::Long,
                format: RoomOptionFormat::FreeForAll,
                item_mode: RoomOptionItemMode::Recommended,
                match_count: DEFAULT_MATCH_COUNT,
                course_selection: RoomOptionCourseSelection::Poll,
                entry_index: 0,
            };
            ServerRoomOptions::Battle(options)
        };
        let state = State::Room;
        Room {
            host_pk,
            karts,
            spectating_karts,
            spectator_count,
            mode_index,
            pack_hash,
            id,
            long_code,
            short_code,
            options,
            state,
            rng: ChaCha20Rng::from_rng(rng),
        }
    }

    fn is_host(&self, client_pk: &PublicKey) -> bool {
        client_pk == &self.host_pk
    }

    fn is_guest(&self, client_pk: &PublicKey) -> bool {
        !self.is_host(client_pk)
    }

    pub fn karts(&self) -> &[Kart] {
        &self.karts
    }

    pub fn spectator_count(&self) -> usize {
        self.spectator_count
    }

    pub fn mode_index(&self) -> ModeIndex {
        self.mode_index
    }

    pub fn pack_hash(&self) -> &[u8] {
        &self.pack_hash
    }

    pub fn id(&self) -> u128 {
        self.id
    }

    pub fn options(&self) -> &ServerRoomOptions {
        &self.options
    }

    pub fn code(&self) -> u64 {
        match self.code_type() {
            RoomOptionCodeType::Long => self.long_code,
            RoomOptionCodeType::Short => self.short_code,
        }
    }

    fn code_type(&self) -> RoomOptionCodeType {
        match &self.options {
            ServerRoomOptions::Race(options) => options.code_type,
            ServerRoomOptions::Battle(options) => options.code_type,
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
            ServerRoomOptions::Race(options) => options.format,
            ServerRoomOptions::Battle(options) => options.format,
        }
    }

    pub fn team_state(&self) -> Option<&ServerTeamStateMain> {
        match &self.state {
            State::Room => None,
            State::Team { state, .. } => Some(state),
            State::Poll { team_state } => team_state.as_ref(),
        }
    }

    pub fn has_room_lock(&self) -> bool {
        match self.state {
            State::Room => false,
            State::Team { .. } => true,
            State::Poll { .. } => true,
        }
    }

    pub fn insert(&mut self, guest_karts: Vec<Kart>) -> Result<bool> {
        if !self.has_room_lock() && self.karts.len() + guest_karts.len() <= 8 {
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
        if self.has_room_lock() {
            return self.spectating_karts.contains_key(client_pk);
        }

        if spectating {
            let karts = self.karts.extract_if(.., |kart| kart.client_pk() == client_pk).collect();
            self.spectating_karts.insert(*client_pk, karts);
        } else if let Entry::Occupied(o) = self.spectating_karts.entry(*client_pk) {
            if o.get().len() + self.karts.len() > 8 {
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
            (ClientRoomOptions::Race(client), ServerRoomOptions::Race(server)) => {
                anyhow::ensure!(self.is_host(client_pk));
                if self.has_room_lock() {
                    anyhow::ensure!(client == *server);
                } else {
                    anyhow::ensure!(client.lap_count <= MAX_LAP_COUNT);
                    anyhow::ensure!(client.match_count >= MIN_MATCH_COUNT);
                    anyhow::ensure!(client.match_count <= MAX_MATCH_COUNT);
                    self.options = ServerRoomOptions::Race(client);
                }
            }
            (ClientRoomOptions::Battle(client), ServerRoomOptions::Battle(server)) => {
                anyhow::ensure!(self.is_host(client_pk));
                if self.has_room_lock() {
                    anyhow::ensure!(client == *server);
                } else {
                    anyhow::ensure!(client.match_count >= MIN_MATCH_COUNT);
                    anyhow::ensure!(client.match_count <= MAX_MATCH_COUNT);
                    self.options = ServerRoomOptions::Battle(client);
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
                    self.state = State::Poll { team_state: None };
                } else {
                    let state = ServerTeamStateMain {
                        teams: vec![0; self.karts.len()],
                        entry_index: 0,
                        continuing: false as u8,
                    };
                    let deadline = Instant::now() + Duration::from_secs(35);
                    self.state = State::Team { state, deadline };
                }
            }
        } else {
            anyhow::ensure!(!continuing);
        }
        Ok(())
    }

    pub fn set_team_state(&mut self, client_pk: &PublicKey, state: ClientTeamState) -> Result<()> {
        anyhow::ensure!(self.team_state().is_some());
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
            State::Room => anyhow::bail!("Invalid room state"),
            State::Team { state, .. } => {
                state.teams = teams;
                state.entry_index = entry_index;
                if continuing != 0 {
                    let mut state = state.clone();
                    self.balance_teams(&mut state.teams);
                    state.continuing = true as u8;
                    self.state = State::Poll { team_state: Some(state) };
                }
            }
            State::Poll { .. } => (),
        }
        Ok(())
    }

    fn set_team_state_guest(&mut self, client_pk: &PublicKey) -> Result<()> {
        anyhow::ensure!(self.is_guest(client_pk));
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

        if self.has_room_lock() {
            anyhow::ensure!(self.karts.iter().any(|kart| present(kart.client_pk())));
        } else {
            let has_host_kart = self.karts.first().map(Kart::client_pk) == Some(&self.host_pk);
            let has_host_spectating_kart = self.spectating_karts.contains_key(&self.host_pk);
            anyhow::ensure!(has_host_kart || has_host_spectating_kart);
        }

        self.spectator_count = self
            .spectating_karts
            .values()
            .flat_map(|karts| karts.iter().map(|kart| kart.players().len()))
            .sum();

        if let State::Team { state, deadline } = &mut self.state
            && Instant::now() >= *deadline
        {
            let mut state = state.clone();
            self.balance_teams(&mut state.teams);
            state.continuing = true as u8;
            self.state = State::Poll { team_state: Some(state) };
        }

        Ok(())
    }
}

impl Drop for Room {
    fn drop(&mut self) {
        debug!("<- {}", self.id);
    }
}

enum State {
    Room,
    Team { deadline: Instant, state: ServerTeamStateMain },
    Poll { team_state: Option<ServerTeamStateMain> },
}
