use std::collections::hash_map::{Entry, HashMap};
use std::net::SocketAddr;
use std::time::Instant;

use anyhow::{Context, Result, anyhow};

use crate::client::Client;
use crate::crypto::PublicKey;
use crate::rooms::Rooms;

pub struct Clients {
    clients: HashMap<PublicKey, Client>,
}

impl Clients {
    pub fn new() -> Clients {
        Clients { clients: HashMap::new() }
    }

    pub fn player_count(&self) -> usize {
        self.clients.values().map(Client::player_count).sum()
    }

    pub fn get(&self, pk: &PublicKey) -> Result<&Client> {
        self.clients.get(pk).context("Client not found")
    }

    pub fn get_mut(&mut self, pk: &PublicKey) -> Result<&mut Client> {
        self.clients.get_mut(pk).context("Client not found")
    }

    pub fn insert(&mut self, now: Instant, addr: SocketAddr, pk: PublicKey) -> Result<&mut Client> {
        let is_full = self.clients.len() >= 1000;
        match self.clients.entry(pk) {
            Entry::Occupied(o) => {
                let client = o.into_mut();
                client.set_addr(addr);
                Ok(client)
            }
            Entry::Vacant(v) if !is_full => {
                let client = Client::new(now, addr, pk);
                Ok(v.insert(client))
            }
            _ => Err(anyhow!("Reached capacity")),
        }
    }

    pub fn update(&mut self, now: Instant, rooms: &mut Rooms) {
        self.clients.retain(|_, client| client.update(now, rooms).is_ok());
    }
}
