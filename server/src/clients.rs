use std::net::SocketAddr;
use std::ops::{Deref, DerefMut};
use std::time::Instant;

use anyhow::{Context, Result, anyhow};
use scc::hash_map::{Entry, HashMap, OccupiedEntry};

use crate::client::Client;
use crate::crypto::PublicKey;
use crate::rooms::Rooms;

pub struct Clients {
    clients: HashMap<PublicKey, Client>,
    count: usize,
    player_count: usize,
}

impl Clients {
    pub fn new() -> Clients {
        Clients { clients: HashMap::new(), count: 0, player_count: 0 }
    }

    pub fn player_count(&self) -> usize {
        self.player_count
    }

    pub fn read<R>(&self, pk: &PublicKey, f: impl FnOnce(&Client) -> R) -> Result<R> {
        self.clients.read_sync(pk, |_, client| f(client)).context("Client not found")
    }

    pub fn get(&self, pk: &PublicKey) -> Result<ClientRef<'_>> {
        self.clients.get_sync(pk).map(ClientRef::new).context("Client not found")
    }

    pub fn insert(&self, now: Instant, addr: SocketAddr, pk: PublicKey) -> Result<ClientRef<'_>> {
        let is_full = self.count >= 1000;
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

    pub fn update(&mut self, now: Instant, rooms: &mut Rooms) {
        self.count = 0;
        self.player_count = 0;
        self.clients.retain_sync(|_, client| {
            let retain = client.update(now, rooms).is_ok();
            if retain {
                self.count += 1;
                self.player_count += client.player_count();
            }
            retain
        });
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
