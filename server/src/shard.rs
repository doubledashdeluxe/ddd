use std::collections::hash_map::{Entry, HashMap};
use std::hash::{BuildHasher, RandomState};
use std::net::{SocketAddr, UdpSocket};
use std::sync::mpsc::{Receiver, SyncSender};
use std::time::Instant;

use anyhow::Result;
use noise_protocol::U8Array;

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::connection::Connection;
use crate::crypto::Key;
use crate::formats::online::FrameRate;
use crate::message::Message;
use crate::rooms::Rooms;

pub fn run(
    server_k: Key,
    socket: UdpSocket,
    message_receiver: Receiver<Message>,
    buffer_sender: SyncSender<Buffer>,
    clients: Clients,
    rooms: Rooms,
) -> Result<()> {
    let shard = Shard {
        server_k,
        socket,
        message_receiver,
        buffer_sender,
        random_state: RandomState::new(),
        connections: HashMap::new(),
        clients,
        rooms,
    };
    shard.run()
}

struct Shard {
    server_k: Key,
    socket: UdpSocket,
    message_receiver: Receiver<Message>,
    buffer_sender: SyncSender<Buffer>,
    random_state: RandomState,
    connections: HashMap<SocketAddr, (bool, Connection)>,
    clients: Clients,
    rooms: Rooms,
}

impl Shard {
    fn run(mut self) -> Result<()> {
        let mut tick_counter = 0;
        loop {
            let message = self.message_receiver.recv()?;
            let now = Instant::now();
            match message {
                Message::Read { addr, buffer } => {
                    self.read(now, tick_counter, addr, buffer.as_slice())?;
                    self.buffer_sender.send(buffer)?;
                }
                Message::Write { frame_rate } => {
                    self.write(now, frame_rate)?;
                    tick_counter += 1;
                }
                Message::Stop => break,
            }
        }
        Ok(())
    }

    fn read(
        &mut self,
        now: Instant,
        tick_counter: u64,
        addr: SocketAddr,
        message: &[u8],
    ) -> Result<()> {
        let is_full = self.connections.len() >= 1000;
        match self.connections.entry(addr) {
            Entry::Occupied(mut o) => {
                let (_, connection) = o.get_mut();
                if connection.read(now, message, &self.clients).is_err() {
                    o.remove();
                }
            }
            Entry::Vacant(v) if !is_full => {
                let Some((client_cookie, message)) = message.split_first_chunk() else {
                    return Ok(());
                };
                let shard_cookie = self.random_state.hash_one((tick_counter >> 12, addr));
                let shard_cookie = shard_cookie.to_be_bytes();
                if *client_cookie != shard_cookie {
                    self.socket.send_to(&shard_cookie, addr)?;
                    return Ok(());
                }

                let connection = Connection::new(self.server_k.clone(), now, addr, message);
                if let Ok(connection) = connection {
                    let retain = true;
                    v.insert((retain, connection));
                }
            }
            Entry::Vacant(_) => (),
        }
        Ok(())
    }

    fn write(&mut self, now: Instant, frame_rate: FrameRate) -> Result<()> {
        let player_count = self.clients.player_count();
        for (addr, (retain, connection)) in &mut self.connections {
            let mut message = [0u8; 512];
            let message_len = connection.write(
                now,
                frame_rate,
                &mut message,
                &self.clients,
                player_count,
                &self.rooms,
            );
            let Ok(message_len) = message_len else {
                *retain = false;
                continue;
            };
            let Some(message_len) = message_len else {
                continue;
            };
            let message = &mut message[..message_len];
            self.socket.send_to(message, addr)?;
        }
        self.connections.retain(|_, (retain, _)| *retain);
        Ok(())
    }
}
