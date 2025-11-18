use std::array;
use std::collections::{self, HashSet};
use std::ops::{Deref, DerefMut};
use std::sync::Arc;

use anyhow::{Context, Result};
use rand::Rng;
use scc::Queue;
use scc::hash_map::{Entry, HashMap, OccupiedEntry};

use crate::crypto::PublicKey;
use crate::formats::online::{FrameRate, ModeIndex};
use crate::kart::Kart;
use crate::pack::Pack;
use crate::room::Room;

#[derive(Clone)]
pub struct Rooms {
    rooms: [Arc<HashMap<u128, Room>>; 2],
    long_code_ids: Arc<HashMap<u64, u128>>,
    short_code_ids: Arc<HashMap<u64, u128>>,
    short_codes: Arc<Queue<u64>>,
}

impl Rooms {
    pub fn new() -> Rooms {
        let rooms = array::from_fn(|_| Arc::new(HashMap::new()));
        let long_code_ids = Arc::new(HashMap::new());
        let short_code_ids = Arc::new(HashMap::new());
        let short_codes: HashSet<_> = (0..1000).collect();
        let short_codes = Arc::new(short_codes.into_iter().collect());
        Rooms { rooms, long_code_ids, short_code_ids, short_codes }
    }

    pub fn read<R>(&self, id: &u128, f: impl FnOnce(&Room) -> R) -> Result<R> {
        let rooms = self.rooms_by_id(id);
        rooms.read_sync(id, |_, room| f(room)).context("Room not found")
    }

    pub fn get(&self, id: &u128) -> Result<RoomRef<'_>> {
        let rooms = self.rooms_by_id(id);
        get(rooms, id)
    }

    pub fn get_by_frame_rate_and_code(
        &self,
        frame_rate: FrameRate,
        code: u64,
    ) -> Result<RoomRef<'_>> {
        let rooms = self.rooms_by_frame_rate(frame_rate);
        let id = self.get_id(code)?;
        let room = get(rooms, &id)?;
        anyhow::ensure!(room.code() == code);
        Ok(room)
    }

    fn get_id(&self, code: u64) -> Result<u128> {
        let ids = if code >> (15 * 3) == 0 { &self.short_code_ids } else { &self.long_code_ids };
        ids.read_sync(&code, |_, id| *id).context("Room ID not found")
    }

    pub fn insert(
        &self,
        frame_rate: FrameRate,
        karts: Vec<Kart>,
        mode_index: ModeIndex,
        pack: Pack,
        rng: &mut impl Rng,
    ) -> Result<u128> {
        anyhow::ensure!(pack.course_count != 0);

        let room_entry = loop {
            let rooms = self.rooms_by_frame_rate(frame_rate);
            let mut id = rng.random();
            id &= !1;
            id |= frame_rate as u128;
            if let Entry::Vacant(v) = rooms.entry_sync(id) {
                break v;
            }
        };

        let long_code_id_entry = loop {
            let code: u64 = rng.random();
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
        let short_code = **self.short_codes.pop().context("Reached capacity")?;
        let room = Room::new(karts, mode_index, pack, id, long_code, short_code, rng);
        room_entry.insert_entry(room);
        long_code_id_entry.insert_entry(id);
        self.short_code_ids.insert_sync(short_code, id).unwrap();
        Ok(id)
    }

    pub fn update(
        &self,
        frame_rate: FrameRate,
        client_room_ids: &collections::HashMap<PublicKey, Option<u128>>,
    ) {
        let rooms = self.rooms_by_frame_rate(frame_rate);
        rooms.retain_sync(|_, room| room.update(client_room_ids).is_ok());
        let retain = |id: &u128| (*id % 2 != frame_rate as u128) || rooms.contains_sync(id);
        self.long_code_ids.retain_sync(|_, id| retain(id));
        self.short_code_ids.retain_sync(|short_code, id| {
            let retain = retain(id);
            if !retain {
                self.short_codes.push(*short_code);
            }
            retain
        });
    }

    fn rooms_by_frame_rate(&self, frame_rate: FrameRate) -> &Arc<HashMap<u128, Room>> {
        &self.rooms[frame_rate as usize]
    }

    fn rooms_by_id(&self, id: &u128) -> &Arc<HashMap<u128, Room>> {
        &self.rooms[(id % 2) as usize]
    }
}

fn get<'a>(rooms: &'a Arc<HashMap<u128, Room>>, id: &u128) -> Result<RoomRef<'a>> {
    rooms.get_sync(id).map(RoomRef::new).context("Room not found")
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
