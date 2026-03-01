use std::iter;
use std::net::UdpSocket;
use std::num::NonZero;
use std::result;
use std::sync::mpsc;
use std::thread::{self, Builder};

use anyhow::Result;
use noise_protocol::U8Array;

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::crypto::Key;
use crate::formats::online::{DEFAULT_PORT, FrameRate};
use crate::listener;
use crate::rooms::Rooms;
use crate::shard;
use crate::updater;

pub fn run(server_k: &Key) -> Result<()> {
    let shard_count = thread::available_parallelism().map_or(1, NonZero::get);
    let socket = UdpSocket::bind(format!("0.0.0.0:{DEFAULT_PORT}"))?;
    let sockets: result::Result<_, _> =
        iter::repeat_with(|| socket.try_clone()).take(shard_count).collect();
    let sockets: Vec<_> = sockets?;
    let (message_senders, message_receivers): (Vec<_>, Vec<_>) =
        iter::repeat_with(|| mpsc::sync_channel(1000)).take(shard_count).unzip();
    let buffer_count = 1000 * shard_count;
    let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffer_count);
    for _ in 0..buffer_count {
        buffer_sender.send(Buffer::new())?;
    }
    let clients = Clients::new();
    let rooms = Rooms::new();

    for (i, (socket, message_receiver)) in sockets.into_iter().zip(message_receivers).enumerate() {
        let server_k = server_k.clone();
        let buffer_sender = buffer_sender.clone();
        let clients = clients.clone();
        let rooms = rooms.clone();
        Builder::new().name(format!("shard/{i}")).spawn(move || {
            shard::run(&server_k, &socket, &message_receiver, &buffer_sender, &clients, &rooms);
        })?;
    }

    for (i, frame_rate) in [FrameRate::SixtyHz, FrameRate::FiftyHz].into_iter().enumerate() {
        let message_senders = message_senders.clone();
        let clients = clients.clone();
        let rooms = rooms.clone();
        Builder::new()
            .name(format!("updater/{i}"))
            .spawn(move || updater::run(&message_senders, &clients, &rooms, frame_rate))?;
    }

    let handle = Builder::new()
        .name("listener".to_owned())
        .spawn(move || listener::run(&socket, &message_senders, &buffer_receiver))?;
    handle.join().unwrap();
    Ok(())
}
