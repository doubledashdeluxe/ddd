use std::iter;
use std::ops::Add;
use std::sync::Arc;
use std::thread::Builder;
use std::time::Duration;

use anyhow::Result;
use arc_swap::ArcSwap;
use log::info;
use noise_protocol::DH;

use ddd_server::config::Config;
use ddd_server::crypto::x25519::X25519;
use ddd_server::flood::counters::Counters;
use ddd_server::flood::mpmc;
use ddd_server::flood::mpsc;
use ddd_server::flood::options;
use ddd_server::logger;
use ddd_server::options::Options;
use ddd_server::server;

fn main() -> Result<()> {
    logger::init()?;

    let flood_options = options::options().run();
    anyhow::ensure!(flood_options.parallelism >= 1);

    let options = Options::default();

    let config = Config::new();
    let config = Arc::new(ArcSwap::from_pointee(config));

    let update = None;
    let update = Arc::new(ArcSwap::from_pointee(update));

    let server_k = X25519::genkey();

    let shards = flood_options.parallelism;
    let buffers_per_shard = config.load().buffers_per_shard;

    let (senders, flood_receivers) = mpmc::channel(shards, buffers_per_shard);
    let senders = senders.into_iter().map(Ok);
    let (flood_senders, receiver) = mpsc::channel(shards, buffers_per_shard);

    server::spawn(options, &config, &update, &server_k, shards, senders, receiver)?;

    let server_pk = X25519::pubkey(&server_k);

    let duration = Duration::from_secs(flood_options.duration);

    let threads: Result<Vec<_>> = iter::zip(flood_senders, flood_receivers)
        .enumerate()
        .map(|(i, (sender, receiver))| {
            Builder::new()
                .name(format!("flood/{i}"))
                .spawn(move || (flood_options.flood)(&sender, &receiver, server_pk, duration, i))
                .map_err(Into::into)
        })
        .collect();
    let threads = threads?;

    let counters: Vec<_> = threads.into_iter().map(|thread| thread.join().unwrap()).collect();

    info!("Thread ↑Packets ↓Packets Operations");
    info!("------ -------- -------- ----------");
    for (i, counters) in counters.iter().enumerate() {
        info!("{i:>6} {counters}");
    }
    info!("------ -------- -------- ----------");
    let counters = counters.into_iter().reduce(Add::add).unwrap_or(Counters::new());
    info!(" Total {counters}");
    info!("  Rate {}", counters / duration.as_secs());

    Ok(())
}
