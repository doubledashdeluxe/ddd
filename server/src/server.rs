use std::net::UdpSocket;
use std::num::NonZero;
use std::panic;
use std::result;
use std::sync::mpsc::{self, SyncSender};
use std::thread::{self, JoinHandle};

use anyhow::Result;
use log::error;
use noise_protocol::U8Array;

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::crypto::Key;
use crate::formats::online::{DEFAULT_PORT, FrameRate};
use crate::listener;
use crate::message::Message;
use crate::rooms::Rooms;
use crate::shard;
use crate::updater;

pub fn run(server_k: Key) -> Result<()> {
    let shard_count = thread::available_parallelism().map_or(1, NonZero::get);
    let socket = UdpSocket::bind(format!("0.0.0.0:{DEFAULT_PORT}"))?;
    let sockets: result::Result<_, _> = (0..shard_count).map(|_| socket.try_clone()).collect();
    let sockets: Vec<_> = sockets?;
    let (message_senders, message_receivers): (Vec<_>, Vec<_>) =
        (0..shard_count).map(|_| mpsc::sync_channel(1000)).unzip();
    let buffer_count = 1000 * shard_count;
    let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffer_count);
    for _ in 0..buffer_count {
        buffer_sender.send(Buffer::new())?;
    }
    let clients = Clients::new();
    let rooms = Rooms::new();

    let mut handles: Vec<_> = sockets
        .into_iter()
        .zip(message_receivers)
        .map(|(socket, message_receiver)| {
            let server_k = server_k.clone();
            let buffer_sender = buffer_sender.clone();
            let clients = clients.clone();
            let rooms = rooms.clone();
            let run =
                || shard::run(server_k, socket, message_receiver, buffer_sender, clients, rooms);
            spawn(message_senders.clone(), run)
        })
        .collect();

    for frame_rate in [FrameRate::SixtyHz, FrameRate::FiftyHz] {
        let run = {
            let message_senders = message_senders.clone();
            let clients = clients.clone();
            let rooms = rooms.clone();
            move || updater::run(message_senders, clients, rooms, frame_rate)
        };
        handles.push(spawn(message_senders.clone(), run));
    }

    let run = {
        let message_senders = message_senders.clone();
        || listener::run(socket, message_senders, buffer_receiver)
    };
    handles.push(spawn(message_senders.clone(), run));

    let results: Vec<_> = handles
        .into_iter()
        .map(|handle| match handle.join() {
            Ok(r) => r,
            Err(e) => panic::resume_unwind(e),
        })
        .collect();
    results.into_iter().collect()
}

fn spawn(
    message_senders: Vec<SyncSender<Message>>,
    f: impl FnOnce() -> Result<()> + Send + 'static,
) -> JoinHandle<Result<()>> {
    thread::spawn(|| {
        let r = f();
        if let Err(e) = &r {
            error!("{e}");
        }
        for message_sender in message_senders {
            let message = Message::Stop;
            let _ = message_sender.send(message);
        }
        r
    })
}
