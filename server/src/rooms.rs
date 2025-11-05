use std::collections::hash_map::{Entry, HashMap};
use std::collections::{HashSet, VecDeque};

use anyhow::{Context, Result};
use orion::util;

use crate::clients::Clients;
use crate::formats::online::ModeIndex;
use crate::kart::Kart;
use crate::room::Room;

pub struct Rooms {
    rooms: HashMap<u128, Room>,
    long_code_ids: HashMap<u64, u128>,
    short_code_ids: HashMap<u64, u128>,
    short_codes: VecDeque<u64>,
}

impl Rooms {
    pub fn new() -> Rooms {
        let rooms = HashMap::new();
        let long_code_ids = HashMap::new();
        let short_code_ids = HashMap::new();
        let short_codes: HashSet<_> = (0..1000).collect();
        let short_codes = short_codes.into_iter().collect();
        Rooms { rooms, long_code_ids, short_code_ids, short_codes }
    }

    pub fn get(&self, id: &u128) -> Result<&Room> {
        self.rooms.get(id).context("Room not found")
    }

    pub fn get_mut(&mut self, id: &u128) -> Result<&mut Room> {
        self.rooms.get_mut(id).context("Room not found")
    }

    pub fn get_mut_by_code(&mut self, code: u64) -> Result<&mut Room> {
        let id = self.get_id(code)?;
        let room = self.get_mut(&id)?;
        anyhow::ensure!(room.code() == code);
        Ok(room)
    }

    fn get_id(&self, code: u64) -> Result<u128> {
        let ids = if code >> (15 * 3) == 0 { &self.short_code_ids } else { &self.long_code_ids };
        ids.get(&code).copied().context("Room ID not found")
    }

    pub fn insert(
        &mut self,
        karts: Vec<Kart>,
        mode_index: ModeIndex,
        pack_hash: Vec<u8>,
    ) -> Result<u128> {
        let room_entry = loop {
            let mut id = [0; 16];
            util::secure_rand_bytes(&mut id)?;
            let id = u128::from_ne_bytes(id);
            if let Entry::Vacant(v) = self.rooms.entry(id) {
                break v;
            }
        };

        let long_code_id_entry = loop {
            let mut code = [0; 8];
            util::secure_rand_bytes(&mut code)?;
            let code = u64::from_ne_bytes(code);
            let code = code >> (64 - 20 * 3);
            if code >> (15 * 3) == 0 {
                continue;
            }
            if let Entry::Vacant(v) = self.long_code_ids.entry(code) {
                break v;
            }
        };

        let id = *room_entry.key();
        let long_code = *long_code_id_entry.key();
        let short_code = self.short_codes.pop_front().context("Reached capacity")?;
        let room = Room::new(karts, mode_index, pack_hash, id, long_code, short_code);
        room_entry.insert(room);
        long_code_id_entry.insert(id);
        self.short_code_ids.insert(short_code, id);
        Ok(id)
    }

    pub fn update(&mut self, clients: &mut Clients) {
        self.rooms.retain(|_, room| room.update(clients).is_ok());
        self.long_code_ids.retain(|_, id| self.rooms.contains_key(id));
        let short_code_ids = self.short_code_ids.extract_if(|_, id| !self.rooms.contains_key(id));
        self.short_codes.extend(short_code_ids.map(|(short_code, _)| short_code));
    }
}
