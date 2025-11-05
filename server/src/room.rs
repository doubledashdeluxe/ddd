use std::collections::hash_map::{Entry, HashMap};

use anyhow::Result;
use log::debug;

use crate::clients::Clients;
use crate::crypto::PublicKey;
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
}

impl Room {
    pub fn new(
        host_karts: Vec<Kart>,
        mode_index: ModeIndex,
        pack_hash: Vec<u8>,
        id: u128,
        long_code: u64,
        short_code: u64,
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
        let state = State::Room { continuing: false };
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

    fn max_team_size(&self) -> usize {
        match self.format() {
            RoomOptionFormat::FreeForAll => 1,
            RoomOptionFormat::TeamsOf2 => 2,
            RoomOptionFormat::TeamsOf4 => 4,
        }
    }

    fn format(&self) -> RoomOptionFormat {
        match &self.options {
            ServerRoomOptions::Race(options) => options.format,
            ServerRoomOptions::Battle(options) => options.format,
        }
    }

    pub fn options(&self) -> &ServerRoomOptions {
        &self.options
    }

    pub fn state(&self) -> &State {
        &self.state
    }

    pub fn has_room_lock(&self) -> bool {
        match self.state {
            State::Room { continuing } => continuing,
            State::Team { .. } => true,
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
        match (options, &self.options, &self.state) {
            (
                ClientRoomOptions::Race(client_options),
                ServerRoomOptions::Race(server_options),
                State::Room { continuing },
            ) => {
                anyhow::ensure!(self.is_host(client_pk));
                if *continuing {
                    anyhow::ensure!(client_options == *server_options);
                } else {
                    anyhow::ensure!(client_options.lap_count <= MAX_LAP_COUNT);
                    anyhow::ensure!(client_options.match_count >= MIN_MATCH_COUNT);
                    anyhow::ensure!(client_options.match_count <= MAX_MATCH_COUNT);
                    self.options = ServerRoomOptions::Race(client_options);
                }
            }
            (
                ClientRoomOptions::Battle(client_options),
                ServerRoomOptions::Battle(server_options),
                State::Room { continuing },
            ) => {
                anyhow::ensure!(self.is_host(client_pk));
                if *continuing {
                    anyhow::ensure!(client_options == *server_options);
                } else {
                    anyhow::ensure!(client_options.match_count >= MIN_MATCH_COUNT);
                    anyhow::ensure!(client_options.match_count <= MAX_MATCH_COUNT);
                    self.options = ServerRoomOptions::Battle(client_options);
                }
            }
            (ClientRoomOptions::None(()), _, _) => {
                anyhow::ensure!(self.is_guest(client_pk))
            }
            _ => anyhow::bail!("Invalid options"),
        }
        Ok(())
    }

    pub fn set_continuing(&mut self, client_pk: &PublicKey, continuing: bool) -> Result<()> {
        if self.is_host(client_pk) {
            if continuing {
                match &mut self.state {
                    State::Room { continuing } => *continuing = true,
                    _ => anyhow::bail!("Unexpected room state"),
                }
            } else {
                anyhow::ensure!(matches!(self.state, State::Room { continuing: false }));
            }
        } else {
            anyhow::ensure!(!continuing);
        }
        Ok(())
    }

    pub fn set_team_state(&mut self, client_pk: &PublicKey, state: ClientTeamState) -> Result<()> {
        anyhow::ensure!(self.has_room_lock());
        let max_team_size = self.max_team_size();
        anyhow::ensure!(max_team_size >= 2);
        let team_count = self.karts.len().div_ceil(max_team_size).max(2) as u8;
        match state {
            ClientTeamState::Host(host) => {
                anyhow::ensure!(self.is_host(client_pk));
                let teams = host.teams;
                let entry_index = host.entry_index;
                anyhow::ensure!(teams.len() == self.karts.len());
                anyhow::ensure!(teams.iter().all(|team| *team < team_count));
                let main = ServerTeamStateMain { teams, entry_index };
                self.state = State::Team { main };
            }
            ClientTeamState::Guest(_) => {
                anyhow::ensure!(self.is_guest(client_pk));
            }
        }
        Ok(())
    }

    pub fn update(&mut self, clients: &Clients) -> Result<()> {
        let present = |client_pk: &PublicKey| {
            clients.get(client_pk).is_ok_and(|client| client.room_id() == Some(self.id))
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

        Ok(())
    }
}

impl Drop for Room {
    fn drop(&mut self) {
        debug!("<- {}", self.id);
    }
}

pub enum State {
    Room { continuing: bool },
    Team { main: ServerTeamStateMain },
}
