use std::collections::BTreeMap;
use std::collections::hash_map::{Entry, HashMap};
use std::hash::{BuildHasher, RandomState};
use std::net::{SocketAddr, UdpSocket};
use std::sync::mpsc::{Receiver, RecvTimeoutError, SyncSender};
use std::time::{Duration, Instant};

use log::error;
use noise_protocol::U8Array;
use rand::{Rng, SeedableRng};

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::connection::Connection;
use crate::crypto::{ChaCha20Rng, Key};
use crate::formats::online::FrameRate;
use crate::message::Message;
use crate::rooms::Rooms;

pub fn run(
    net_sim: bool,
    server_k: &Key,
    socket: &UdpSocket,
    message_receiver: &Receiver<Message>,
    buffer_sender: &SyncSender<Buffer>,
    clients: &Clients,
    rooms: &Rooms,
) -> ! {
    let link = Link {
        net_sim,
        socket,
        message_receiver,
        buffer_sender,
        buffers: vec![Buffer::new(); 5000],
        events: BTreeMap::new(),
        event_index: 0,
        rng: ChaCha20Rng::from_os_rng(),
    };
    let shard = Shard {
        server_k,
        link,
        random_state: RandomState::new(),
        connections: HashMap::new(),
        clients,
        rooms,
    };
    shard.run()
}

struct Shard<'a> {
    server_k: &'a Key,
    link: Link<'a>,
    random_state: RandomState,
    connections: HashMap<SocketAddr, Connection>,
    clients: &'a Clients,
    rooms: &'a Rooms,
}

impl Shard<'_> {
    fn run(mut self) -> ! {
        let mut tick_counter = 0;
        let mut message = None;
        loop {
            let now = Instant::now();
            match message {
                Some(Message::Read { buffer, addr }) => {
                    self.read(now, tick_counter, addr, buffer.as_slice());
                    self.link.send(buffer);
                }
                Some(Message::Write { frame_rate }) => {
                    self.write(now, frame_rate);
                    tick_counter += 1;
                }
                None => (),
            }
            message = self.link.recv(now);
        }
    }

    fn read(&mut self, now: Instant, tick_counter: u64, addr: SocketAddr, message: &[u8]) {
        let is_full = self.connections.len() >= 1000;
        match self.connections.entry(addr) {
            Entry::Occupied(mut o) => {
                let connection = o.get_mut();
                if connection.read(now, message, self.clients).is_err() {
                    o.remove();
                }
            }
            Entry::Vacant(v) if !is_full => {
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

    fn write(&mut self, now: Instant, frame_rate: FrameRate) {
        let player_count = self.clients.player_count();
        self.connections.retain(|addr, connection| {
            let mut message = [0u8; 512];
            let message_len = connection.write(
                now,
                frame_rate,
                &mut message,
                self.clients,
                player_count,
                self.rooms,
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

struct Link<'a> {
    net_sim: bool,
    socket: &'a UdpSocket,
    message_receiver: &'a Receiver<Message>,
    buffer_sender: &'a SyncSender<Buffer>,
    buffers: Vec<Buffer>,
    events: BTreeMap<(Instant, usize), Event>,
    event_index: usize,
    rng: ChaCha20Rng,
}

impl Link<'_> {
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
            Message::Read { buffer, addr } if self.net_sim => {
                let event = Event::Read { buffer, addr };
                self.insert_event(now, event);
                None
            }
            _ => Some(message),
        }
    }

    fn send(&self, buffer: Buffer) {
        self.buffer_sender.send(buffer).unwrap();
    }

    fn send_to(&mut self, now: Instant, buf: &[u8], addr: SocketAddr) {
        if self.net_sim {
            if let Some(mut buffer) = self.buffers.pop() {
                buffer.copy_from(buf);
                let event = Event::Write { buffer, addr };
                self.insert_event(now, event);
            }
        } else if let Err(e) = self.socket.send_to(buf, addr) {
            error!("{e}");
        }
    }

    fn insert_event(&mut self, now: Instant, event: Event) {
        if self.rng.random_bool(0.2) {
            match event {
                Event::Read { buffer, .. } => self.send(buffer),
                Event::Write { buffer, .. } => self.buffers.push(buffer),
            }
        } else {
            let delay = 50;
            let jitter = 15;
            let delay = delay - jitter + self.rng.random_range(..jitter * 2u64);
            let instant = now + Duration::from_millis(delay);
            let index = self.event_index;
            self.events.insert((instant, index), event);
            self.event_index += 1;
        }
    }
}

enum Event {
    Read { buffer: Buffer, addr: SocketAddr },
    Write { buffer: Buffer, addr: SocketAddr },
}
