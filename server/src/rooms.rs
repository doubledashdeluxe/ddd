use std::collections::{HashSet, VecDeque};
use std::ops::{Deref, DerefMut};

use anyhow::{Context, Result};
use orion::util;
use scc::hash_map::{Entry, HashMap, OccupiedEntry};

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

    pub fn read<R>(&self, id: &u128, f: impl FnOnce(&Room) -> R) -> Result<R> {
        self.rooms.read_sync(id, |_, room| f(room)).context("Room not found")
    }

    pub fn get(&self, id: &u128) -> Result<RoomRef<'_>> {
        self.rooms.get_sync(id).map(RoomRef::new).context("Room not found")
    }

    pub fn get_by_code(&self, code: u64) -> Result<RoomRef<'_>> {
        let id = self.get_id(code)?;
        let room = self.get(&id)?;
        anyhow::ensure!(room.code() == code);
        Ok(room)
    }

    fn get_id(&self, code: u64) -> Result<u128> {
        let ids = if code >> (15 * 3) == 0 { &self.short_code_ids } else { &self.long_code_ids };
        ids.read_sync(&code, |_, id| *id).context("Room ID not found")
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
            if let Entry::Vacant(v) = self.rooms.entry_sync(id) {
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
            if let Entry::Vacant(v) = self.long_code_ids.entry_sync(code) {
                break v;
            }
        };

        let id = *room_entry.key();
        let long_code = *long_code_id_entry.key();
        let short_code = self.short_codes.pop_front().context("Reached capacity")?;
        let room = Room::new(karts, mode_index, pack_hash, id, long_code, short_code);
        room_entry.insert_entry(room);
        long_code_id_entry.insert_entry(id);
        self.short_code_ids.insert_sync(short_code, id).unwrap();
        Ok(id)
    }

    pub fn update(&mut self, clients: &Clients) {
        self.rooms.retain_sync(|_, room| room.update(clients).is_ok());
        self.long_code_ids.retain_sync(|_, id| self.rooms.contains_sync(id));
        self.short_code_ids.retain_sync(|short_code, id| {
            let retain = self.rooms.contains_sync(id);
            if !retain {
                self.short_codes.push_back(*short_code);
            }
            retain
        });
    }
}

pub struct RoomRef<'a> {
    entry: OccupiedEntry<'a, u128, Room>,
}

impl RoomRef<'_> {
    pub fn new(entry: OccupiedEntry<'_, u128, Room>) -> RoomRef<'_> {
        RoomRef { entry }
    }
}

impl Deref for RoomRef<'_> {
    type Target = Room;

    fn deref(&self) -> &Room {
        self.entry.get()
    }
}

impl DerefMut for RoomRef<'_> {
    fn deref_mut(&mut self) -> &mut Room {
        self.entry.get_mut()
    }
}
