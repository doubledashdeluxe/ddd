use std::collections;
use std::net::SocketAddr;
use std::ops::{Deref, DerefMut};
use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::time::Instant;

use anyhow::{Context, Result, anyhow};
use rand::Rng;
use scc::hash_map::{Entry, HashMap, OccupiedEntry};

use crate::client::Client;
use crate::crypto::PublicKey;
use crate::rooms::Rooms;

#[derive(Clone, Default)]
pub struct Clients {
    clients: Arc<HashMap<PublicKey, Client>>,
    count: Arc<AtomicUsize>,
    player_count: Arc<AtomicUsize>,
}

impl Clients {
    pub fn new() -> Clients {
        Clients {
            clients: Arc::new(HashMap::new()),
            count: Arc::new(AtomicUsize::new(0)),
            player_count: Arc::new(AtomicUsize::new(0)),
        }
    }

    pub fn player_count(&self) -> usize {
        self.player_count.load(Ordering::Relaxed)
    }

    pub fn read<R>(&self, pk: &PublicKey, f: impl FnOnce(&Client) -> R) -> Result<R> {
        self.clients.read_sync(pk, |_, client| f(client)).context("Client not found")
    }

    pub fn get(&self, pk: &PublicKey) -> Result<ClientRef<'_>> {
        self.clients.get_sync(pk).map(ClientRef::new).context("Client not found")
    }

    pub fn insert(&self, now: Instant, addr: SocketAddr, pk: PublicKey) -> Result<ClientRef<'_>> {
        let is_full = self.count.load(Ordering::Relaxed) >= 1000;
        let entry = match self.clients.entry_sync(pk) {
            Entry::Occupied(mut o) => {
                o.get_mut().set_addr(addr);
                o
            }
            Entry::Vacant(v) if !is_full => {
                let client = Client::new(now, addr, pk);
                v.insert_entry(client)
            }
            _ => return Err(anyhow!("Reached capacity")),
        };
        Ok(ClientRef { entry })
    }

    pub fn update(
        &self,
        now: Instant,
        client_room_ids: &mut collections::HashMap<PublicKey, Option<u128>>,
        rooms: &Rooms,
        rng: &mut impl Rng,
    ) {
        client_room_ids.clear();
        let mut count = 0;
        let mut player_count = 0;
        self.clients.retain_sync(|pk, client| {
            let retain = client.update(now, rooms, rng).is_ok();
            if retain {
                count += 1;
                player_count += client.player_count();
                client_room_ids.insert(*pk, client.room_id());
            }
            retain
        });
        self.count.store(count, Ordering::Relaxed);
        self.player_count.store(player_count, Ordering::Relaxed);
    }
}

pub struct ClientRef<'a> {
    entry: OccupiedEntry<'a, PublicKey, Client>,
}

impl ClientRef<'_> {
    fn new(entry: OccupiedEntry<'_, PublicKey, Client>) -> ClientRef<'_> {
        ClientRef { entry }
    }
}

impl Deref for ClientRef<'_> {
    type Target = Client;

    fn deref(&self) -> &Client {
        self.entry.get()
    }
}

impl DerefMut for ClientRef<'_> {
    fn deref_mut(&mut self) -> &mut Client {
        self.entry.get_mut()
    }
}
