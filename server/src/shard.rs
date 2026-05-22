use std::collections::BTreeMap;
use std::collections::hash_map::{Entry, HashMap};
use std::hash::{BuildHasher, RandomState};
use std::net::{SocketAddr, UdpSocket};
use std::sync::mpsc::{Receiver, RecvTimeoutError, SyncSender};
use std::time::{Duration, Instant};

use arc_swap::Cache;
use log::error;
use noise_protocol::U8Array;
use rand::RngExt;

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::config::{Config, SharedConfig};
use crate::connection::Connection;
use crate::crypto::{ChaCha20Rng, Key};
use crate::formats::online::FrameRate;
use crate::message::Message;
use crate::options::NetSimOptions;
use crate::rooms::Rooms;

pub struct Shard {
    config: SharedConfig,
    server_k: Key,
    link: Link,
    random_state: RandomState,
    connections: HashMap<SocketAddr, Connection>,
    clients: Clients,
    client_slots: usize,
    rooms: Rooms,
}

impl Shard {
    pub fn new(
        net_sim_options: NetSimOptions,
        config: SharedConfig,
        server_k: Key,
        socket: UdpSocket,
        message_receiver: Receiver<Message>,
        buffer_sender: SyncSender<Buffer>,
        clients: Clients,
        rooms: Rooms,
    ) -> Self {
        let link = Link {
            drops: 1.0 - (1.0 - net_sim_options.drops).sqrt(),
            latency: net_sim_options.latency / 2,
            jitter: net_sim_options.jitter / 2,
            socket,
            message_receiver,
            buffer_sender,
            buffers: vec![Buffer::new(); config.load().buffers_per_shard * 5],
            events: BTreeMap::new(),
            event_index: 0,
            rng: rand::make_rng(),
        };
        Self {
            config,
            server_k,
            link,
            random_state: RandomState::new(),
            connections: HashMap::new(),
            clients,
            client_slots: 0,
            rooms,
        }
    }

    pub fn run(mut self) -> ! {
        let mut config = Cache::new(self.config.clone());
        let mut tick_counter = 0;
        let mut message = None;
        loop {
            let now = Instant::now();
            let config = config.load();
            match message {
                Some(Message::Read { buffer, addr }) => {
                    self.read(now, config, tick_counter, addr, buffer.as_slice());
                    self.link.send(buffer);
                }
                Some(Message::Write { frame_rate, client_slots }) => {
                    self.write(now, config, frame_rate);
                    self.client_slots = client_slots;
                    tick_counter += 1;
                }
                None => (),
            }
            message = self.link.recv(now);
        }
    }

    fn read(
        &mut self,
        now: Instant,
        config: &Config,
        tick_counter: u64,
        addr: SocketAddr,
        message: &[u8],
    ) {
        let connection_count = self.connections.len();
        match self.connections.entry(addr) {
            Entry::Occupied(mut o) => {
                let connection = o.get_mut();
                if connection.read(now, message, &self.clients, &mut self.client_slots).is_err() {
                    o.remove();
                }
            }
            Entry::Vacant(v) if connection_count < config.max_connections_per_shard => {
                let Some((client_cookie, message)) = message.split_first_chunk() else {
                    return;
                };
                let shard_cookie = self.random_state.hash_one((tick_counter >> 12, addr));
                let shard_cookie = shard_cookie.to_be_bytes();
                if *client_cookie != shard_cookie {
                    self.link.send_to(now, &shard_cookie, addr);
                    return;
                }

                let connection = Connection::new(self.server_k.clone(), now, addr, message);
                if let Ok(connection) = connection {
                    v.insert(connection);
                }
            }
            Entry::Vacant(_) => (),
        }
    }

    fn write(&mut self, now: Instant, config: &Config, frame_rate: FrameRate) {
        let player_count = self.clients.player_count();
        self.connections.retain(|addr, connection| {
            let mut message = [0u8; 512];
            let message_len = connection.write(
                now,
                config,
                frame_rate,
                &mut message,
                &self.clients,
                player_count,
                &self.rooms,
            );
            match message_len {
                Ok(Some(message_len)) => {
                    let message = &mut message[..message_len];
                    self.link.send_to(now, message, *addr);
                    true
                }
                Ok(None) => true,
                Err(_) => false,
            }
        });
    }
}

struct Link {
    drops: f64,
    latency: u64,
    jitter: u64,
    socket: UdpSocket,
    message_receiver: Receiver<Message>,
    buffer_sender: SyncSender<Buffer>,
    buffers: Vec<Buffer>,
    events: BTreeMap<(Instant, usize), Event>,
    event_index: usize,
    rng: ChaCha20Rng,
}

impl Link {
    fn recv(&mut self, now: Instant) -> Option<Message> {
        for (_, event) in self.events.extract_if(..(now, usize::MAX), |_, _| true) {
            match event {
                Event::Read { buffer, addr } => return Some(Message::Read { buffer, addr }),
                Event::Write { buffer, addr } => {
                    if let Err(e) = self.socket.send_to(buffer.as_slice(), addr) {
                        error!("{e}");
                    }
                    self.buffers.push(buffer);
                }
            }
        }
        let timeout = self
            .events
            .keys()
            .next()
            .and_then(|(instant, _)| instant.checked_duration_since(now))
            .unwrap_or(Duration::MAX);
        let message = match self.message_receiver.recv_timeout(timeout) {
            Err(RecvTimeoutError::Timeout) => return None,
            message => message.unwrap(),
        };
        match message {
            Message::Read { buffer, .. } if self.dropped() => {
                self.send(buffer);
                None
            }
            Message::Read { buffer, addr } => {
                if let Some(event_key) = self.event_key(now) {
                    let event = Event::Read { buffer, addr };
                    self.events.insert(event_key, event);
                    None
                } else {
                    Some(Message::Read { buffer, addr })
                }
            }
            Message::Write { .. } => Some(message),
        }
    }

    fn send(&self, buffer: Buffer) {
        self.buffer_sender.send(buffer).unwrap();
    }

    fn send_to(&mut self, now: Instant, buf: &[u8], addr: SocketAddr) {
        if self.dropped() {
            return;
        }
        if let Some(event_key) = self.event_key(now) {
            if let Some(mut buffer) = self.buffers.pop() {
                buffer.copy_from(buf);
                let event = Event::Write { buffer, addr };
                self.events.insert(event_key, event);
            }
        } else if let Err(e) = self.socket.send_to(buf, addr) {
            error!("{e}");
        }
    }

    fn dropped(&mut self) -> bool {
        self.rng.random_bool(self.drops)
    }

    fn event_key(&mut self, now: Instant) -> Option<(Instant, usize)> {
        let latency = self
            .latency
            .checked_add(self.rng.random_range(..=self.jitter * 2))?
            .checked_sub(self.jitter)
            .filter(|latency| *latency > 0)?;
        let instant = now + Duration::from_millis(latency);
        let index = self.event_index;
        self.event_index += 1;
        Some((instant, index))
    }
}

enum Event {
    Read { buffer: Buffer, addr: SocketAddr },
    Write { buffer: Buffer, addr: SocketAddr },
}
