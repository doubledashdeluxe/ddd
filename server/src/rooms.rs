use std::array;
use std::collections;
use std::ops::{Deref, DerefMut};
use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};

use anyhow::{Context, Result};
use heapless::Vec;
use rand::{Rng, RngExt};
use scc::hash_map::{Entry, HashMap, OccupiedEntry, VacantEntry};
use scc::{HashSet, Queue};

use crate::crypto::PublicKey;
use crate::formats::online::*;
use crate::kart::Kart;
use crate::mmr::Mmr;
use crate::pack::Pack;
use crate::room::{CodePair, Room};

#[derive(Clone)]
pub struct Rooms {
    rooms: [Arc<HashMap<u128, Room>>; 2],
    counts: [Arc<AtomicUsize>; 2],
    search_rooms: [Arc<HashMap<Search, SearchRooms>>; 2],
    mode_player_counts: [Arc<[AtomicUsize; MODE_INDEX_COUNT]>; 2],
    long_code_ids: Arc<HashMap<u64, u128>>,
    short_code_ids: Arc<HashMap<u64, u128>>,
    short_codes: Arc<Queue<u64>>,
}

impl Rooms {
    pub fn new() -> Self {
        let short_codes: collections::HashSet<_> = (0..32768).collect();
        Self {
            rooms: array::from_fn(|_| Arc::new(HashMap::new())),
            counts: array::from_fn(|_| Arc::new(0.into())),
            search_rooms: array::from_fn(|_| Arc::new(HashMap::new())),
            mode_player_counts: array::from_fn(|_| Arc::new(array::from_fn(|_| 0.into()))),
            long_code_ids: Arc::new(HashMap::new()),
            short_code_ids: Arc::new(HashMap::new()),
            short_codes: Arc::new(short_codes.into_iter().collect()),
        }
    }

    pub fn count(&self, frame_rate: FrameRate) -> usize {
        self.counts[frame_rate as usize].load(Ordering::Relaxed)
    }

    pub fn mode_player_counts(&self, frame_rate: FrameRate) -> [usize; MODE_INDEX_COUNT] {
        let counts = &self.mode_player_counts[frame_rate as usize];
        counts.each_ref().map(|count| count.load(Ordering::Relaxed))
    }

    pub fn search_player_count(&self, frame_rate: FrameRate, search: &Search) -> usize {
        let search_rooms = &self.search_rooms[frame_rate as usize];
        search_rooms.read_sync(search, |_, rooms| rooms.player_count).unwrap_or(0)
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
        anyhow::ensure!(room.code() == Some(code));
        Ok(room)
    }

    fn get_id(&self, code: u64) -> Result<u128> {
        let ids = if code >> (15 * 3) == 0 { &self.short_code_ids } else { &self.long_code_ids };
        ids.read_sync(&code, |_, id| *id).context("Room ID not found")
    }

    pub fn search(
        &self,
        room_slots: &mut usize,
        frame_rate: FrameRate,
        karts: &[Kart],
        search: Search,
        rng: &mut impl Rng,
    ) -> Result<RoomRef<'_>> {
        anyhow::ensure!(search.pack.course_count != 0);
        let mmr = karts.mmr();
        let rooms = self.rooms_by_frame_rate(frame_rate);
        let search_rooms = &self.search_rooms[frame_rate as usize];
        let ids = search_rooms.entry_sync(search.clone()).or_default().ids.clone();
        let mut best_room = None;
        ids.iter_mut_sync(|id| {
            let room = get(rooms, &id).ok().filter(|room| {
                let room_kart_count = room.karts().len() + room.spectating_kart_count();
                room_kart_count + karts.len() <= MAX_ROOM_KART_COUNT
            });
            if let Some(room) = room {
                let diff = room.mmr().abs_diff(mmr);
                if best_room.as_ref().is_none_or(|(_, best_diff)| diff < *best_diff) {
                    best_room = Some((room, diff));
                }
            }
            true
        });
        if let Some((room, _)) = best_room {
            Ok(room)
        } else {
            anyhow::ensure!(*room_slots != 0);
            *room_slots -= 1;
            let room_entry = self.vacant_room_entry(frame_rate, rng);
            let id = *room_entry.key();
            let room = Room::new_worldwide(search.mode_index, search.pack, id, search.format, rng);
            let room = room_entry.insert_entry(room);
            ids.insert_sync(id).unwrap();
            Ok(RoomRef::new(room))
        }
    }

    pub fn insert(
        &self,
        room_slots: &mut usize,
        frame_rate: FrameRate,
        karts: Vec<Kart, MAX_CLIENT_KART_COUNT>,
        mode_index: ModeIndex,
        pack: Pack,
        rng: &mut impl Rng,
    ) -> Result<u128> {
        anyhow::ensure!(pack.course_count != 0);
        anyhow::ensure!(*room_slots != 0);
        *room_slots -= 1;
        let room_entry = self.vacant_room_entry(frame_rate, rng);
        let long_code_id_entry = self.long_code_id_entry(rng);
        let id = *room_entry.key();
        let code_pair = CodePair {
            long: *long_code_id_entry.key(),
            short: **self.short_codes.pop().context("Reached capacity")?,
        };
        let room = Room::new_personal(karts, mode_index, pack, id, code_pair, rng);
        room_entry.insert_entry(room);
        long_code_id_entry.insert_entry(id);
        self.short_code_ids.insert_sync(code_pair.short, id).unwrap();
        Ok(id)
    }

    pub fn update(
        &self,
        frame_rate: FrameRate,
        client_room_ids: &collections::HashMap<PublicKey, Option<u128>>,
    ) {
        let rooms = self.rooms_by_frame_rate(frame_rate);
        let mut count = 0;
        rooms.retain_sync(|_, room| {
            let retain = room.update(client_room_ids).is_ok();
            if retain {
                count += 1;
            }
            retain
        });
        self.counts[frame_rate as usize].store(count, Ordering::Relaxed);
        let search_rooms = &self.search_rooms[frame_rate as usize];
        let mut mode_player_counts = [0; MODE_INDEX_COUNT];
        search_rooms.retain_sync(|search, SearchRooms { ids, player_count }| {
            *player_count = 0;
            ids.retain_sync(|id| {
                let room = get(rooms, id);
                if let Ok(room) = &room {
                    let room_player_count = room.player_count();
                    mode_player_counts[search.mode_index as usize] += room_player_count;
                    *player_count += room_player_count;
                }
                room.is_ok()
            });
            !ids.is_empty()
        });
        let next = mode_player_counts.into_iter();
        let curr = self.mode_player_counts[frame_rate as usize].iter();
        for (next, curr) in next.zip(curr) {
            curr.store(next, Ordering::Relaxed);
        }
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

    const fn rooms_by_frame_rate(&self, frame_rate: FrameRate) -> &Arc<HashMap<u128, Room>> {
        &self.rooms[frame_rate as usize]
    }

    fn rooms_by_id(&self, id: &u128) -> &Arc<HashMap<u128, Room>> {
        &self.rooms[(id % 2) as usize]
    }

    fn vacant_room_entry(
        &self,
        frame_rate: FrameRate,
        rng: &mut impl Rng,
    ) -> VacantEntry<'_, u128, Room> {
        loop {
            let rooms = self.rooms_by_frame_rate(frame_rate);
            let mut id = rng.random();
            id &= !1;
            id |= frame_rate as u128;
            if let Entry::Vacant(v) = rooms.entry_sync(id) {
                break v;
            }
        }
    }

    fn long_code_id_entry(&self, rng: &mut impl Rng) -> VacantEntry<'_, u64, u128> {
        loop {
            let code: u64 = rng.random();
            let code = code >> (64 - 20 * 3);
            if code >> (15 * 3) == 0 {
                continue;
            }
            if let Entry::Vacant(v) = self.long_code_ids.entry_sync(code) {
                break v;
            }
        }
    }
}

fn get<'a>(rooms: &'a Arc<HashMap<u128, Room>>, id: &u128) -> Result<RoomRef<'a>> {
    rooms.get_sync(id).map(RoomRef::new).context("Room not found")
}

pub struct RoomRef<'a> {
    entry: OccupiedEntry<'a, u128, Room>,
}

impl<'a> RoomRef<'a> {
    pub const fn new(entry: OccupiedEntry<'a, u128, Room>) -> Self {
        Self { entry }
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

#[derive(Clone, Debug, Eq, Hash, PartialEq)]
pub struct Search {
    pub mode_index: ModeIndex,
    pub pack: Pack,
    pub format: RoomOptionFormat,
}

#[derive(Default)]
struct SearchRooms {
    ids: Arc<HashSet<u128>>,
    player_count: usize,
}
