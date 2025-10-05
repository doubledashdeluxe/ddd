use std::collections::hash_map::{Entry, HashMap};
use std::hash::{BuildHasher, RandomState};
use std::io::ErrorKind;
use std::net::{SocketAddr, UdpSocket};
use std::time::{Duration, Instant};

use anyhow::Result;
use noise_protocol::U8Array;

use crate::client::Client;
use crate::connection::Connection;
use crate::crypto::sensitive::Sensitive;

pub struct Server {
    server_k: Sensitive<[u8; 32]>,
    socket: UdpSocket,
    random_state: RandomState,
    connections: HashMap<SocketAddr, Connection>,
    clients: HashMap<[u8; 32], Client>,
}

impl Server {
    pub fn try_new(server_k: Sensitive<[u8; 32]>) -> Result<Server> {
        let socket = UdpSocket::bind("0.0.0.0:3549")?;
        let random_state = RandomState::new();
        let connections = HashMap::new();
        let clients = HashMap::new();
        let server = Server { server_k, random_state, socket, connections, clients };
        Ok(server)
    }

    pub fn run(mut self) -> Result<()> {
        let mut next_tick = Instant::now();
        let mut tick_counter = 0;
        loop {
            let now = Instant::now();
            match next_tick.checked_duration_since(now) {
                Some(duration) if !duration.is_zero() => self.read(now, duration, tick_counter)?,
                _ => {
                    self.write(now)?;
                    next_tick += Duration::from_nanos(16683333);
                    tick_counter += 1;
                }
            }
        }
    }

    fn read(&mut self, now: Instant, duration: Duration, tick_counter: u64) -> Result<()> {
        self.socket.set_read_timeout(Some(duration))?;
        let mut message = [0u8; 512];
        let (message_len, addr) = match self.socket.recv_from(&mut message) {
            Err(e) if e.kind() == ErrorKind::WouldBlock || e.kind() == ErrorKind::TimedOut => {
                return Ok(())
            }
            r => r?,
        };
        let message = &mut message[..message_len];
        let is_full = self.connections.len() >= 1000;
        match self.connections.entry(addr) {
            Entry::Occupied(mut o) => {
                let connection = o.get_mut();
                if connection.read(now, message, &mut self.clients).is_err() {
                    o.remove();
                }
            }
            Entry::Vacant(v) if !is_full => {
                let Some((client_cookie, message)) = message.split_first_chunk() else {
                    return Ok(());
                };
                let server_cookie = self.random_state.hash_one((tick_counter >> 12, addr));
                let server_cookie = server_cookie.to_be_bytes();
                if *client_cookie != server_cookie {
                    self.socket.send_to(&server_cookie, addr)?;
                    return Ok(());
                }

                let connection = Connection::try_new(self.server_k.clone(), now, addr, message);
                if let Ok(connection) = connection {
                    v.insert(connection);
                }
            }
            _ => (),
        }
        Ok(())
    }

    fn write(&mut self, now: Instant) -> Result<()> {
        let mut connections = HashMap::with_capacity(self.connections.len());
        for (addr, mut connection) in self.connections.drain() {
            let mut message = [0u8; 512];
            let message_len = match connection.write(now, &mut message, &mut self.clients) {
                Ok(message_len) => message_len,
                Err(_) => continue,
            };
            if let Some(message_len) = message_len {
                let message = &mut message[..message_len];
                self.socket.send_to(message, addr)?;
            }
            connections.insert(addr, connection);
        }
        self.connections = connections;
        self.clients.retain(|_, client| !client.has_expired(now));
        Ok(())
    }
}
