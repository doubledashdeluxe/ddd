use std::collections::hash_map::{Entry, HashMap};

use anyhow::Result;
use log::debug;

use crate::client::Client;
use crate::formats::online::*;
use crate::kart::Kart;

pub struct Room {
    host_pk: [u8; 32],
    karts: Vec<Kart>,
    spectating_karts: HashMap<[u8; 32], Vec<Kart>>,
    spectator_count: usize,
    mode_index: ModeIndex,
    is_race: bool,
    pack_hash: Vec<u8>,
    id: u128,
    long_code: u64,
    short_code: u64,
    options: ServerRoomOptions,
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
        Room {
            host_pk,
            karts,
            spectating_karts,
            spectator_count,
            mode_index,
            is_race,
            pack_hash,
            id,
            long_code,
            short_code,
            options,
        }
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
        let code_type = match &self.options {
            ServerRoomOptions::Race(options) => options.code_type,
            ServerRoomOptions::Battle(options) => options.code_type,
        };
        match code_type {
            RoomOptionCodeType::Long => self.long_code,
            RoomOptionCodeType::Short => self.short_code,
        }
    }

    pub fn options(&self) -> &ServerRoomOptions {
        &self.options
    }

    pub fn insert(&mut self, guest_karts: Vec<Kart>) -> Result<bool> {
        if self.karts.len() + guest_karts.len() <= 8 {
            self.karts.extend(guest_karts);
            Ok(false)
        } else {
            anyhow::ensure!(self.spectating_karts.len() < 1000);
            let guest_pk = *guest_karts[0].client_pk();
            self.spectating_karts.insert(guest_pk, guest_karts);
            Ok(true)
        }
    }

    pub fn set_spectating(&mut self, client_pk: &[u8; 32], spectating: bool) -> bool {
        if spectating {
            let karts = self.karts.extract_if(.., |kart| kart.client_pk() == client_pk).collect();
            self.spectating_karts.insert(*client_pk, karts);
        } else {
            if let Entry::Occupied(o) = self.spectating_karts.entry(*client_pk) {
                if o.get().len() + self.karts.len() > 8 {
                    return true;
                }
                let karts = o.remove();
                if client_pk == &self.host_pk {
                    self.karts.splice(0..0, karts);
                } else {
                    self.karts.extend(karts);
                }
            }
        }
        spectating
    }

    pub fn set_options(&mut self, client_pk: &[u8; 32], options: ClientRoomOptions) -> Result<()> {
        match options {
            ClientRoomOptions::Race(options) => {
                anyhow::ensure!(client_pk == &self.host_pk);
                anyhow::ensure!(self.is_race);
                anyhow::ensure!(options.lap_count <= MAX_LAP_COUNT);
                anyhow::ensure!(options.match_count >= MIN_MATCH_COUNT);
                anyhow::ensure!(options.match_count <= MAX_MATCH_COUNT);
                self.options = ServerRoomOptions::Race(options);
            }
            ClientRoomOptions::Battle(options) => {
                anyhow::ensure!(client_pk == &self.host_pk);
                anyhow::ensure!(!self.is_race);
                anyhow::ensure!(options.match_count >= MIN_MATCH_COUNT);
                anyhow::ensure!(options.match_count <= MAX_MATCH_COUNT);
                self.options = ServerRoomOptions::Battle(options);
            }
            ClientRoomOptions::None(_) => {
                anyhow::ensure!(client_pk != &self.host_pk);
            }
        }
        Ok(())
    }

    pub fn update(&mut self, clients: &HashMap<[u8; 32], Client>) -> Result<()> {
        self.karts.retain(|kart| {
            let client_pk = kart.client_pk();
            clients.get(client_pk).map(|client| client.room_id() == Some(self.id)).unwrap_or(false)
        });

        let has_host_kart = self.karts.get(0).map(|kart| kart.client_pk()) == Some(&self.host_pk);
        let has_host_spectating_kart = self.spectating_karts.contains_key(&self.host_pk);
        anyhow::ensure!(has_host_kart || has_host_spectating_kart);

        self.spectator_count = self
            .spectating_karts
            .values()
            .flat_map(|karts| karts.iter().map(|kart| kart.server_kart().players.len()))
            .sum();

        Ok(())
    }
}

impl Drop for Room {
    fn drop(&mut self) {
        debug!("<- {}", self.id);
    }
}
