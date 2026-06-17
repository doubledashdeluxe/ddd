use std::array;
use std::clone::Clone;
use std::collections::hash_map::{Entry, HashMap};
use std::iter;
use std::ops::BitOr;
use std::time::{Duration, Instant};

use anyhow::{Context, Result};
use heapless::linear_map::{self, LinearMap};
use log::debug;
use rand::seq::SliceRandom;
use rand::{Rng, RngExt, SeedableRng};

use crate::config::Config;
use crate::crypto::{ChaCha20Rng, PublicKey};
use crate::formats::online::*;
use crate::item;
use crate::kart::Kart;
use crate::mmr::Mmr;
use crate::pack::Pack;

pub struct Room {
    host_pk: Option<PublicKey>,
    karts: heapless::Vec<Kart, MAX_ROOM_KART_COUNT>,
    spectating_karts: HashMap<PublicKey, heapless::Vec<Kart, MAX_CLIENT_KART_COUNT>>,
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
        let host_karts = heapless::Vec::new();
        let (max_kart_count, max_client_kart_count, match_count) = match format {
            RoomOptionFormat::Duel => (2, 1, 5),
            _ => (MAX_ROOM_KART_COUNT, MAX_CLIENT_KART_COUNT, 0),
        };
        let code_pair = None;
        Self::new(
            host_karts,
            max_kart_count,
            max_client_kart_count,
            mode_index,
            pack,
            id,
            code_pair,
            format,
            match_count,
            rng,
        )
    }

    pub fn new_personal(
        host_karts: heapless::Vec<Kart, MAX_CLIENT_KART_COUNT>,
        mode_index: ModeIndex,
        pack: Pack,
        id: u128,
        code_pair: CodePair,
        rng: &mut impl Rng,
    ) -> Self {
        let code_pair = Some(code_pair);
        Self::new(
            host_karts,
            MAX_ROOM_KART_COUNT,
            MAX_CLIENT_KART_COUNT,
            mode_index,
            pack,
            id,
            code_pair,
            RoomOptionFormat::FreeForAll,
            DEFAULT_MATCH_COUNT,
            rng,
        )
    }

    fn new(
        host_karts: heapless::Vec<Kart, MAX_CLIENT_KART_COUNT>,
        max_kart_count: usize,
        max_client_kart_count: usize,
        mode_index: ModeIndex,
        pack: Pack,
        id: u128,
        code_pair: Option<CodePair>,
        format: RoomOptionFormat,
        match_count: u8,
        rng: &mut impl Rng,
    ) -> Self {
        debug!("-> {id}");
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
                format,
                engine_size: RoomOptionEngineSize::Large,
                item_mode: RoomOptionItemMode::Recommended,
                lap_count: 0,
                match_count,
                course_selection: RoomOptionCourseSelection::Poll,
                entry_index: 0,
            };
            ServerRoomOptions::RaceOptions(options)
        } else {
            let options = RoomOptionsBattle {
                battle: (),
                code_type: RoomOptionCodeType::Long,
                format,
                match_count,
                course_selection: RoomOptionCourseSelection::Poll,
                entry_index: 0,
            };
            ServerRoomOptions::BattleOptions(options)
        };
        Self {
            host_pk: host_karts.first().map(Kart::client_pk).copied(),
            karts: host_karts.into_iter().collect(),
            spectating_karts: HashMap::new(),
            spectator_count: 0,
            max_kart_count,
            max_client_kart_count,
            mode_index,
            pack,
            id,
            code_pair,
            options,
            state: State::new_room(),
            rng: ChaCha20Rng::from_rng(rng),
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

    pub const fn spectator_count(&self) -> usize {
        self.spectator_count
    }

    pub const fn mode_index(&self) -> ModeIndex {
        self.mode_index
    }

    pub const fn pack(&self) -> &Pack {
        &self.pack
    }

    pub const fn id(&self) -> u128 {
        self.id
    }

    pub const fn options(&self) -> &ServerRoomOptions {
        &self.options
    }

    pub const fn code(&self) -> Option<u64> {
        match (&self.code_pair, self.code_type()) {
            (Some(code_pair), RoomOptionCodeType::Long) => Some(code_pair.long),
            (Some(code_pair), RoomOptionCodeType::Short) => Some(code_pair.short),
            (None, _) => None,
        }
    }

    const fn code_type(&self) -> RoomOptionCodeType {
        match &self.options {
            ServerRoomOptions::RaceOptions(options) => options.code_type,
            ServerRoomOptions::BattleOptions(options) => options.code_type,
        }
    }

    const fn is_duel(&self) -> bool {
        matches!(self.format(), RoomOptionFormat::Duel)
    }

    const fn has_teams(&self) -> bool {
        matches!(self.format(), RoomOptionFormat::TeamsOf2 | RoomOptionFormat::TeamsOf4)
    }

    fn team_count(&self) -> u8 {
        let max_team_size = match self.format() {
            RoomOptionFormat::TeamsOf2 => 2,
            RoomOptionFormat::TeamsOf4 => 4,
            _ => 1,
        };
        self.karts.len().div_ceil(max_team_size).max(2) as u8
    }

    fn balance_teams(&mut self, teams: &mut [u8]) {
        let mut team_sizes = [0; MAX_TEAM_COUNT];
        for team in teams.iter() {
            team_sizes[*team as usize] += 1;
        }

        let kart_count = teams.len();
        let mut karts: [_; MAX_ROOM_KART_COUNT] = array::from_fn(|i| i);
        karts[..kart_count].shuffle(&mut self.rng);

        let team_count = self.team_count() as usize;
        let mut other_teams: [_; MAX_TEAM_COUNT] = array::from_fn(|i| i);
        other_teams[..team_count].shuffle(&mut self.rng);

        let max_team_size = kart_count.div_ceil(team_count);
        for kart in &karts[..kart_count] {
            let team = &mut teams[*kart];
            let team_size = team_sizes[*team as usize];
            if team_size <= max_team_size {
                continue;
            }

            for other_team in &other_teams[..team_count] {
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

    const fn format(&self) -> RoomOptionFormat {
        match &self.options {
            ServerRoomOptions::RaceOptions(options) => options.format,
            ServerRoomOptions::BattleOptions(options) => options.format,
        }
    }

    const fn course_selection(&self) -> RoomOptionCourseSelection {
        match &self.options {
            ServerRoomOptions::RaceOptions(options) => options.course_selection,
            ServerRoomOptions::BattleOptions(options) => options.course_selection,
        }
    }

    pub const fn has_team_state(&self) -> bool {
        match &self.state {
            State::Room => false,
            State::Team { .. } => true,
            State::Poll { team_state, .. } => team_state.is_some(),
            State::Race { team_state, .. } => team_state.is_some(),
        }
    }

    pub const fn team_state(&self) -> Option<&ServerTeamStateMain> {
        match &self.state {
            State::Room => None,
            State::Team { state, .. } => Some(state),
            State::Poll { team_state, .. } => team_state.as_ref(),
            State::Race { team_state, .. } => team_state.as_ref(),
        }
    }

    pub const fn has_poll_state(&self) -> bool {
        match &self.state {
            State::Room => false,
            State::Team { .. } => false,
            State::Poll { .. } => true,
            State::Race { .. } => true,
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

    pub const fn has_race_state(&self) -> bool {
        match &self.state {
            State::Room => false,
            State::Team { .. } => false,
            State::Poll { .. } => false,
            State::Race { .. } => true,
        }
    }

    pub fn race_state(&self, client_pk: &PublicKey, frame: u16) -> Option<ServerRaceStateMain> {
        match &self.state {
            State::Room => None,
            State::Team { .. } => None,
            State::Poll { .. } => None,
            State::Race { states, .. } => {
                let mut state = states.get(frame as usize).or_else(|| states.last())?.clone();
                state.frame = states.len() as u16 - 1;
                state.client_frame = frame;
                let mut j = 0;
                for (i, kart) in self.karts.iter().enumerate() {
                    if state.kart_flags & 1 << i == 0 {
                        continue;
                    }
                    if !self.is_duel() && kart.client_pk() != client_pk {
                        state.karts[j].item_frames.fill(MIN_CLIENT_FRAME);
                        state.karts[j].item_ids.fill(ItemId::None);
                    }
                    j += 1;
                }
                Some(state)
            }
        }
    }

    pub const fn has_room_lock(&self) -> bool {
        match self.state {
            State::Room => false,
            State::Team { .. } => true,
            State::Poll { .. } => true,
            State::Race { .. } => true,
        }
    }

    pub fn insert(
        &mut self,
        config: &Config,
        guest_karts: heapless::Vec<Kart, MAX_CLIENT_KART_COUNT>,
    ) -> Result<bool> {
        anyhow::ensure!(guest_karts.len() <= self.max_client_kart_count);
        if !self.has_room_lock() && self.karts.len() + guest_karts.len() <= self.max_kart_count {
            self.karts.extend(guest_karts);
            Ok(false)
        } else {
            anyhow::ensure!(self.spectating_karts.len() < config.max_spectators_per_room);
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
            let mut karts = heapless::Vec::new();
            self.karts.retain(|kart| {
                let retain = kart.client_pk() != client_pk;
                if !retain {
                    karts.push(kart.clone()).unwrap();
                }
                retain
            });
            self.spectating_karts.insert(*client_pk, karts);
        } else if let Entry::Occupied(o) = self.spectating_karts.entry(*client_pk) {
            if o.get().len() + self.karts.len() > self.max_kart_count {
                return true;
            }

            let karts = o.remove();
            if self.is_host(client_pk) {
                for kart in karts.into_iter().rev() {
                    self.karts.insert(0, kart).unwrap();
                }
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
                anyhow::ensure!(self.is_guest(client_pk));
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
                if self.is_duel() {
                    for kart in self.karts.drain(2..) {
                        let client_pk = kart.client_pk();
                        let karts = self.spectating_karts.entry(*client_pk).or_default();
                        karts.push(kart).unwrap();
                    }
                }
                if self.has_teams() {
                    let deadline = Instant::now() + Duration::from_secs(35);
                    self.state = State::new_team(self.karts.len(), deadline);
                } else {
                    self.state = State::new_poll(None);
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
        host: ClientTeamStateHost,
    ) -> Result<()> {
        anyhow::ensure!(self.has_team_state());
        anyhow::ensure!(self.is_host(client_pk));
        let ClientTeamStateHost { teams, entry_index, continuing } = host;
        anyhow::ensure!(teams.len() == self.karts.len());
        let team_count = self.team_count();
        anyhow::ensure!(teams.iter().all(|team| *team < team_count));
        let State::Team { state, .. } = &mut self.state else { return Ok(()) };
        state.teams = teams;
        state.entry_index = entry_index;
        if continuing != 0 {
            let mut state = state.clone();
            self.balance_teams(&mut state.teams);
            state.continuing = u8::from(true);
            self.state = State::new_poll(Some(state));
        }
        Ok(())
    }

    fn set_team_state_guest(&self, client_pk: &PublicKey) -> Result<()> {
        anyhow::ensure!(self.has_team_state());
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
        anyhow::ensure!(self.has_poll_state());
        Ok(())
    }

    fn set_poll_state_ready(
        &mut self,
        client_pk: &PublicKey,
        ready: ClientPollStateReady,
    ) -> Result<()> {
        anyhow::ensure!(self.has_poll_state());
        let ClientPollStateReady { karts: client_karts, course_index } = ready;
        let kart_indices: heapless::Vec<_, MAX_CLIENT_KART_COUNT> = self
            .karts
            .iter()
            .enumerate()
            .filter(|(_, kart)| kart.client_pk() == client_pk)
            .map(|(i, _)| i as u8)
            .collect();
        anyhow::ensure!(client_karts.len() == kart_indices.len());
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
                anyhow::ensure!(course_index.is_some() == !kart_indices.is_empty());
            }
            RoomOptionCourseSelection::Host => {
                anyhow::ensure!(course_index.is_some() == is_host);
            }
            RoomOptionCourseSelection::Random => {
                anyhow::ensure!(course_index.is_none());
            }
        }
        let State::Poll { team_state, state, karts: server_karts, host_course_index, .. } =
            &mut self.state
        else {
            return Ok(());
        };
        let kart_indices = kart_indices.into_iter();
        let is_host_selection = course_selection == RoomOptionCourseSelection::Host;
        for (client_kart, kart_index) in client_karts.into_iter().zip(kart_indices) {
            let linear_map::Entry::Vacant(v) = server_karts.entry(kart_index) else {
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
            state.kart_indices.push(kart_index).unwrap();
            v.insert(server_kart).unwrap();
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
                &self.karts,
                &mut self.rng,
            );
        }
        Ok(())
    }

    pub fn set_race(&mut self, client_pk: &PublicKey, race: ClientStateRace) -> Result<()> {
        anyhow::ensure!(self.has_race_state());
        let ClientStateRace { frame: client_frame, karts: mut client_karts, item_counts } = race;
        anyhow::ensure!(client_frame >= MIN_CLIENT_FRAME);
        let kart_indices: heapless::Vec<_, MAX_CLIENT_KART_COUNT> = self
            .karts
            .iter()
            .enumerate()
            .filter(|(_, kart)| kart.client_pk() == client_pk)
            .map(|(i, _)| i)
            .collect();
        anyhow::ensure!(client_karts.len() == kart_indices.len());
        let State::Race { poll_state, inputs, karts, states, lightning_available_frame, .. } =
            &mut self.state
        else {
            return Ok(());
        };
        let server_frame = states.len() as u16;
        if i32::from(client_frame) - i32::from(server_frame) > MAX_KART_INPUT_COUNT as i32 {
            return Ok(());
        }
        let kart_indices = || kart_indices.iter().copied();
        for (client_kart, kart_index) in client_karts.iter_mut().zip(kart_indices()) {
            let kart = karts[kart_index].as_ref();
            let kart_frame = kart.map_or(MIN_CLIENT_FRAME, |kart| kart.kart_frame);
            if client_frame <= kart_frame {
                return Ok(());
            }
            let min_input_count = client_frame - kart_frame;
            let client_inputs = &client_kart.inputs;
            let inputs = &inputs[kart_index];
            anyhow::ensure!(client_inputs.len() == inputs.len());
            anyhow::ensure!(client_inputs[0].len() >= min_input_count as usize);
            for [ci0, ci1] in client_inputs.array_windows() {
                anyhow::ensure!(ci0.len() == ci1.len());
            }
            for (i, item_frame) in client_kart.item_frames.iter().enumerate() {
                let kart_item_frame = kart.map_or(MIN_CLIENT_FRAME, |kart| kart.item_frames[i]);
                anyhow::ensure!(*item_frame >= kart_item_frame);
                anyhow::ensure!(*item_frame <= client_frame);
            }
            let kart_item_event_counter = kart.map_or(0, |kart| kart.item_event_counter);
            let item_event_count = client_kart
                .item_event_counter
                .checked_sub(kart_item_event_counter)
                .context("Invalid item event counter")? as usize;
            client_kart.item_events.truncate(item_event_count);
            for item_event in &client_kart.item_events {
                anyhow::ensure!(item_event.event_stick_y >= MIN_STICK_Y);
                anyhow::ensure!(item_event.event_stick_y <= MAX_STICK_Y);
            }
            anyhow::ensure!((client_kart.rank as usize) < self.karts.len());
        }
        for item_count in &item_counts {
            anyhow::ensure!(*item_count <= 64);
        }
        for (mut client_kart, kart_index) in client_karts.into_iter().zip(kart_indices()) {
            let client_inputs = client_kart.inputs;
            let inputs = &mut inputs[kart_index];
            for (client_inputs, inputs) in client_inputs.iter().zip(inputs.iter_mut()) {
                let input_count = (client_frame - MIN_CLIENT_FRAME) as usize - inputs.len();
                let input_offset = client_inputs.len() - input_count;
                inputs.extend(client_inputs[input_offset..].iter());
            }
            let inputs = client_inputs.iter().map(|inputs| *inputs.last().unwrap()).collect();
            let kart = &karts[kart_index];
            let mut item_ids = kart.as_ref().map_or([ItemId::None; 2], |kart| kart.item_ids);
            for (i, item_frame) in client_kart.item_frames.iter_mut().enumerate() {
                let kart_item_frame =
                    kart.as_ref().map_or(MIN_CLIENT_FRAME, |kart| kart.item_frames[i]);
                if *item_frame >= kart_item_frame + 50 {
                    let item_mode = match &self.options {
                        ServerRoomOptions::RaceOptions(options) => options.item_mode,
                        ServerRoomOptions::BattleOptions(_) => RoomOptionItemMode::Recommended,
                    };
                    let poll_kart = poll_state
                        .karts
                        .iter()
                        .find(|kart| kart.kart_index as usize == kart_index)
                        .unwrap();
                    let character_ids = poll_kart.character_ids;
                    let mut item_counts = item_counts;
                    for kart in &*karts {
                        let Some(kart) = kart else {
                            continue;
                        };
                        for item_id in &kart.item_ids {
                            let base_item_id = item_id.base();
                            if base_item_id != ItemId::None {
                                let count = item_id.count();
                                item_counts[base_item_id as usize] += count;
                            }
                        }
                    }
                    let lightning_available =
                        lightning_available_frame.is_some_and(|frame| server_frame >= frame);
                    item_ids[i] = item::choose(
                        self.karts.len(),
                        self.mode_index,
                        item_mode,
                        character_ids[i],
                        character_ids[i ^ 1],
                        client_kart.rank,
                        item_ids[i ^ 1],
                        item_counts,
                        lightning_available,
                        &mut self.rng,
                    );
                } else {
                    *item_frame = kart_item_frame;
                }
            }
            if let Some(kart) = kart {
                while kart.item_events.len() + client_kart.item_events.len() > MAX_ITEM_EVENT_COUNT
                {
                    client_kart.item_event_counter -= 1;
                    client_kart.item_events.remove(0);
                }
            }
            for item_event in &mut client_kart.item_events {
                item_event.event_frame = 0;
                let item_id =
                    item_ids.iter_mut().find(|item_id| **item_id == item_event.event_item_id);
                if let Some(item_id) = item_id {
                    if *item_id == ItemId::Lightning {
                        *lightning_available_frame = server_frame.checked_add((10 + 30) * 60);
                    }
                    *item_id = ItemId::None;
                } else {
                    item_event.event_item_id = ItemId::None;
                }
            }
            let mut item_events = client_kart.item_events;
            if let Some(kart) = kart {
                item_events.extend(kart.item_events.clone());
            }
            karts[kart_index] = Some(ServerRaceKart {
                kart_frame: client_frame,
                inputs,
                driver: client_kart.driver,
                pos_x: client_kart.pos_x,
                pos_y: client_kart.pos_y,
                pos_z: client_kart.pos_z,
                angle: client_kart.angle,
                vel_x: client_kart.vel_x,
                vel_z: client_kart.vel_z,
                item_event_counter: client_kart.item_event_counter,
                item_frames: client_kart.item_frames,
                item_ids,
                item_events,
            });
        }
        Ok(())
    }

    pub fn update(&mut self, client_room_ids: &HashMap<PublicKey, Option<u128>>) -> Result<()> {
        let present = |client_pk: &PublicKey| {
            client_room_ids.get(client_pk).is_some_and(|room_id| *room_id == Some(self.id))
        };

        if !self.has_room_lock() {
            self.karts.retain(|kart| present(kart.client_pk()));
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
                        self.karts.extend(karts.drain(..));
                    }
                    retain
                });
                if self.karts.array_windows().any(|[k0, k1]| k0.client_pk() != k1.client_pk()) {
                    if self.has_teams() {
                        let deadline = Instant::now();
                        self.state = State::new_team(self.karts.len(), deadline);
                    } else {
                        self.state = State::new_poll(None);
                    }
                }
            }
            State::Team { state, deadline } if Instant::now() >= *deadline => {
                let mut state = state.clone();
                self.balance_teams(&mut state.teams);
                state.continuing = u8::from(true);
                self.state = State::new_poll(Some(state));
            }
            State::Poll { team_state, state, karts, host_course_index, deadline }
                if Instant::now() >= *deadline =>
            {
                for (kart_index, kart) in self.karts.iter().enumerate() {
                    let kart_index = kart_index as u8;
                    let is_host = Some(kart.client_pk()) == self.host_pk.as_ref();
                    let linear_map::Entry::Vacant(v) = karts.entry(kart_index) else {
                        continue;
                    };
                    let character_ids = array::from_fn(|_| self.rng.random());
                    let kart_id = loop {
                        let kart_id: KartId = self.rng.random();
                        if kart_id.compatible(&character_ids) {
                            break kart_id;
                        }
                    };
                    let course_index = self.rng.random_range(..self.pack.course_count);
                    let kart = ServerPollKart { kart_index, character_ids, kart_id, course_index };
                    state.kart_indices.push(kart_index).unwrap();
                    v.insert(kart).unwrap();
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
                    &self.karts,
                    &mut self.rng,
                );
            }
            State::Race { karts, states, .. } => {
                let kart_flags = karts
                    .iter()
                    .enumerate()
                    .map(|(i, kart)| u8::from(kart.is_some()) << i)
                    .fold(0, BitOr::bitor);
                let state = ServerRaceStateMain {
                    frame: states.len() as u16,
                    client_frame: states.len() as u16,
                    kart_flags,
                    karts: karts.iter().filter_map(Clone::clone).collect(),
                };
                states.push(state);
                for kart in karts {
                    let Some(kart) = kart else { continue };
                    kart.item_events.retain_mut(|item_event| {
                        item_event.event_frame += 1;
                        item_event.event_frame < MAX_KART_INPUT_COUNT as u8
                    });
                }
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

enum State {
    Room,
    Team {
        state: ServerTeamStateMain,
        deadline: Instant,
    },
    Poll {
        team_state: Option<ServerTeamStateMain>,
        state: ServerPollStatePending,
        karts: LinearMap<u8, ServerPollKart, MAX_ROOM_KART_COUNT>,
        host_course_index: Option<u8>,
        deadline: Instant,
    },
    Race {
        team_state: Option<ServerTeamStateMain>,
        poll_state: ServerPollStateReady,
        inputs: heapless::Vec<Inputs, MAX_ROOM_KART_COUNT>,
        karts: heapless::Vec<Option<ServerRaceKart>, MAX_ROOM_KART_COUNT>,
        states: Vec<ServerRaceStateMain>,
        lightning_available_frame: Option<u16>,
    },
}

impl State {
    const fn new_room() -> Self {
        Self::Room
    }

    fn new_team(kart_count: usize, deadline: Instant) -> Self {
        let state = ServerTeamStateMain {
            teams: iter::repeat_n(0, kart_count).collect(),
            entry_index: 0,
            continuing: u8::from(false),
        };
        Self::Team { state, deadline }
    }

    fn new_poll(team_state: Option<ServerTeamStateMain>) -> Self {
        Self::Poll {
            team_state,
            state: ServerPollStatePending { kart_indices: heapless::Vec::new() },
            karts: LinearMap::new(),
            host_course_index: None,
            deadline: Instant::now() + Duration::from_secs(35),
        }
    }

    fn new_race(
        team_state: Option<ServerTeamStateMain>,
        state: &ServerPollStatePending,
        server_karts: &mut LinearMap<u8, ServerPollKart, MAX_ROOM_KART_COUNT>,
        host_course_index: Option<u8>,
        karts: &heapless::Vec<Kart, MAX_ROOM_KART_COUNT>,
        rng: &mut impl Rng,
    ) -> Self {
        let inputs = karts
            .iter()
            .map(|kart| iter::repeat_n(vec![], kart.players().len()).collect())
            .collect();
        let mut karts: heapless::Vec<_, _> = state
            .kart_indices
            .iter()
            .map(|kart_index| server_karts.remove(kart_index).unwrap())
            .collect();
        let kart_count = karts.len();
        let selected_kart_index = rng.random_range(..kart_count as u8);
        if let Some(host_course_index) = host_course_index {
            karts[selected_kart_index as usize].course_index = host_course_index;
        }
        Self::Race {
            team_state,
            poll_state: ServerPollStateReady { karts, selected_kart_index },
            inputs,
            karts: iter::repeat_n(None, kart_count).collect(),
            states: vec![],
            lightning_available_frame: Some(MIN_CLIENT_FRAME + 30 * 60),
        }
    }
}

type Inputs = heapless::Vec<Vec<u16>, MAX_KART_PLAYER_COUNT>;
