use std::fmt::Write as _;
use std::fs::File;
use std::io::{Read, Write};

use anyhow::Result;
use log::info;
use noise_protocol::{DH, U8Array};

use crate::crypto::x25519::X25519;
use crate::formats::version;

mod buffer;
mod client;
mod clients;
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
mod server;
mod shard;
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
    info!("Server public key: {server_pk}");

    server::run(options.net_sim, &server_k)
}
