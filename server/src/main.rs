use std::env;
use std::fmt::Write as _;
use std::fs::{self, File};
use std::io::{Read, Write as _};
use std::iter;
use std::net::UdpSocket;
use std::num::NonZero;
use std::path::Path;
use std::sync::Arc;
use std::thread;

use anyhow::Result;
use arc_swap::ArcSwap;
use log::{LevelFilter, error, info};
use noise_protocol::{DH, U8Array};

use ddd_server::config::Config;
use ddd_server::courses::Courses;
use ddd_server::crypto::x25519::X25519;
use ddd_server::formats::online::DEFAULT_PORT;
use ddd_server::formats::version;
use ddd_server::logger;
use ddd_server::options;
use ddd_server::server;
use ddd_server::sighup::Sighup;
use ddd_server::systemd;
use ddd_server::update::Update;

fn main() -> Result<()> {
    logger::init(LevelFilter::Trace)?;
    let version = version::VERSION;
    info!("Double Dash Deluxe Server [{version}]");

    let options = options::options().run();
    anyhow::ensure!((0.0..=1.0).contains(&options.net_sim.drops));
    anyhow::ensure!(options.net_sim.latency <= 1000);
    anyhow::ensure!(options.net_sim.jitter <= 1000);

    let config_path = env::var_os("CONFIGURATION_DIRECTORY");
    let config_path = config_path.as_ref().map(Path::new);

    let credentials_path = env::var_os("CREDENTIALS_DIRECTORY");
    let credentials_path = credentials_path.as_ref().map_or_else(|| "run".as_ref(), Path::new);
    fs::create_dir_all(credentials_path)?;

    info!("Loading configuration...");
    let config = Config::read(config_path, credentials_path)?;
    let config = Arc::new(ArcSwap::from_pointee(config));
    info!("Loaded configuration.");

    info!("Loading update...");
    let update = Update::read(config_path)?;
    let update = Arc::new(ArcSwap::from_pointee(update));
    info!("Loaded update.");

    info!("Loading courses...");
    let courses = Courses::read(config_path)?;
    let courses = Arc::new(ArcSwap::from_pointee(courses));
    info!("Loaded courses.");

    let path = credentials_path.join("private-key");
    let server_k = if let Ok(mut file) = File::open(&path) {
        let mut server_k = <X25519 as DH>::Key::new();
        anyhow::ensure!(file.metadata()?.len() == server_k.len() as u64);
        file.read_exact(server_k.as_mut())?;
        server_k
    } else {
        let server_k = X25519::genkey();
        let mut file = File::create_new(&path)?;
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
    info!("Shards: {shards}");

    let socket = UdpSocket::bind(("0.0.0.0", DEFAULT_PORT))?;
    let senders = iter::repeat_with(|| Ok(socket.try_clone()?)).take(shards);
    let receiver = socket.try_clone()?;

    let state_path = env::var_os("STATE_DIRECTORY");
    let state_path = state_path.as_ref().map_or_else(|| "run".as_ref(), Path::new);
    fs::create_dir_all(state_path)?;

    server::spawn(
        options, &config, &update, &courses, &server_k, shards, senders, receiver, state_path,
    )?;

    let mut sighup = Sighup::new()?;
    systemd::notify_ready()?;
    loop {
        for () in sighup.wait() {
            systemd::notify_reloading()?;

            reload(|| Config::read(config_path, credentials_path), &config, "config");
            reload(|| Update::read(config_path), &update, "update");
            reload(|| Courses::read(config_path), &courses, "courses");

            systemd::notify_ready()?;
        }
    }
}

fn reload<T>(reload: impl Fn() -> Result<T>, x: &Arc<ArcSwap<T>>, name: &str) {
    info!("Reloading {name}...");
    match reload() {
        Ok(new) => {
            x.store(Arc::new(new));
            info!("Reloaded {name}.");
        }
        Err(e) => error!("Failed to reload {name}: {e}"),
    }
}
