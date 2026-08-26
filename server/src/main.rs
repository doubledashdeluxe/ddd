use std::fmt::Write as _;
use std::fs::{self, File};
use std::io::{Read, Write as _};
use std::iter;
use std::net::UdpSocket;
use std::num::NonZero;
use std::sync::Arc;
use std::thread;

use anyhow::Result;
use arc_swap::ArcSwap;
use log::{debug, error, info};
use noise_protocol::{DH, U8Array};

use ddd_server::config::Config;
use ddd_server::courses::Courses;
use ddd_server::crypto::x25519::X25519;
use ddd_server::formats::online::DEFAULT_PORT;
use ddd_server::formats::version;
use ddd_server::logger;
use ddd_server::options;
use ddd_server::server;
use ddd_server::sighup;
use ddd_server::update::Update;

fn main() -> Result<()> {
    logger::init()?;
    let version = version::VERSION;
    info!("Double Dash Deluxe Server [{version}]");

    let options = options::options().run();
    anyhow::ensure!((0.0..=1.0).contains(&options.net_sim.drops));
    anyhow::ensure!(options.net_sim.latency <= 1000);
    anyhow::ensure!(options.net_sim.jitter <= 1000);

    fs::create_dir_all("run")?;

    debug!("Loading configuration...");
    let config = Config::read()?;
    let config = Arc::new(ArcSwap::from_pointee(config));
    debug!("Loaded configuration.");

    debug!("Loading update...");
    let update = Update::read()?;
    let update = Arc::new(ArcSwap::from_pointee(update));
    debug!("Loaded update.");

    debug!("Loading courses...");
    let courses = Courses::read()?;
    let courses = Arc::new(ArcSwap::from_pointee(courses));
    debug!("Loaded courses.");

    let server_k = if let Ok(mut file) = File::open("run/k.bin") {
        let mut server_k = <X25519 as DH>::Key::new();
        anyhow::ensure!(file.metadata()?.len() == server_k.len() as u64);
        file.read_exact(server_k.as_mut())?;
        server_k
    } else {
        let server_k = X25519::genkey();
        let mut file = File::create_new("run/k.bin")?;
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

    let socket = UdpSocket::bind(("0.0.0.0", DEFAULT_PORT))?;
    let senders = iter::repeat_with(|| Ok(socket.try_clone()?)).take(shards);
    let receiver = socket.try_clone()?;

    server::spawn(
        options, &config, &update, &courses, &server_k, shards, senders, receiver, "run",
    )?;

    for () in sighup::sighup()? {
        reload(Config::read, &config, "config");
        reload(Update::read, &update, "update");
        reload(Courses::read, &courses, "courses");
    }

    unreachable!()
}

fn reload<T>(reload: fn() -> Result<T>, x: &Arc<ArcSwap<T>>, name: &str) {
    debug!("Reloading {name}...");
    match reload() {
        Ok(new) => {
            x.store(Arc::new(new));
            debug!("Reloaded {name}.");
        }
        Err(e) => error!("Failed to reload {name}: {e}"),
    }
}
