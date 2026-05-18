use std::fmt::Write as _;
use std::fs::File;
use std::io::{Read, Write};
use std::net::UdpSocket;
use std::num::NonZero;
use std::sync::Arc;
use std::sync::mpsc;
use std::thread::{self, Builder};

use anyhow::Result;
use arc_swap::ArcSwap;
use log::{debug, error, info};
use noise_protocol::{DH, U8Array};

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::config::Config;
use crate::crypto::x25519::X25519;
use crate::formats::online::{DEFAULT_PORT, FrameRate};
use crate::formats::version;
use crate::listener::Listener;
use crate::rooms::Rooms;
use crate::shard::Shard;
use crate::updater::Updater;

mod buffer;
mod client;
mod clients;
mod config;
mod connection;
mod crypto;
mod formats;
mod item;
mod item_weights;
mod kart;
mod listener;
mod logger;
mod message;
mod mmr;
mod options;
mod pack;
mod player;
mod room;
mod rooms;
mod shard;
mod sighup;
mod updater;
mod weight;

fn main() -> Result<()> {
    logger::init()?;
    let version = version::VERSION;
    info!("Double Dash Deluxe Server [{version}]");

    let options = options::options().run();
    anyhow::ensure!((0.0..=1.0).contains(&options.net_sim.drops));
    anyhow::ensure!(options.net_sim.latency <= 1000);
    anyhow::ensure!(options.net_sim.jitter <= 1000);

    debug!("Loading configuration...");
    let config = Config::read()?;
    let config = Arc::new(ArcSwap::from_pointee(config));
    debug!("Loaded configuration.");

    let server_k = if let Ok(mut file) = File::open("k.bin") {
        let mut server_k = <X25519 as DH>::Key::new();
        anyhow::ensure!(file.metadata()?.len() == server_k.len() as u64);
        file.read_exact(server_k.as_mut())?;
        server_k
    } else {
        let server_k = X25519::genkey();
        let mut file = File::create_new("k.bin")?;
        file.write_all(server_k.as_slice())?;
        server_k
    };

    let server_pk = X25519::pubkey(&server_k);
    let server_pk: Result<_> =
        server_pk.into_iter().try_fold(String::new(), |mut server_pk, byte| {
            write!(server_pk, "{byte:02x?}")?;
            Ok(server_pk)
        });
    let server_pk = server_pk?;
    info!("Public key: {server_pk}");

    let shards = match config.load().shards {
        0 => thread::available_parallelism().map_or(1, NonZero::get),
        shards => shards,
    };
    debug!("Shards: {shards}");

    let socket = UdpSocket::bind(format!("0.0.0.0:{DEFAULT_PORT}"))?;

    let buffers_per_shard = config.load().buffers_per_shard;
    let buffers = buffers_per_shard * shards;
    let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffers);
    for _ in 0..buffers {
        buffer_sender.send(Buffer::new())?;
    }

    let clients = Clients::new();
    let rooms = Rooms::new();

    let message_senders: Result<_> = (0..shards)
        .map(|i| {
            let (message_sender, message_receiver) = mpsc::sync_channel(buffers_per_shard);
            let shard = Shard::new(
                options.net_sim,
                config.clone(),
                server_k.clone(),
                socket.try_clone()?,
                message_receiver,
                buffer_sender.clone(),
                clients.clone(),
                rooms.clone(),
            );
            Builder::new().name(format!("shard/{i}")).spawn(|| shard.run())?;
            Ok(message_sender)
        })
        .collect();
    let message_senders: Vec<_> = message_senders?;

    for (i, frame_rate) in [FrameRate::SixtyHz, FrameRate::FiftyHz].into_iter().enumerate() {
        let updater = Updater::new(
            config.clone(),
            message_senders.clone(),
            clients.clone(),
            rooms.clone(),
            frame_rate,
        );
        Builder::new().name(format!("updater/{i}")).spawn(move || updater.run())?;
    }

    let listener = Listener::new(socket, message_senders, buffer_receiver);
    Builder::new().name("listener".to_owned()).spawn(move || listener.run())?;

    for () in sighup::sighup()? {
        debug!("Reloading configuration...");
        match Config::read() {
            Ok(new_config) => config.store(Arc::new(new_config)),
            Err(e) => error!("Failed to reload configuration: {e}"),
        }
        debug!("Reloaded configuration.");
    }

    unreachable!()
}
