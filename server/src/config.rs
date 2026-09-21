use std::convert::Infallible;
use std::fs;
use std::io::ErrorKind;
use std::str::FromStr;
use std::sync::Arc;

use anyhow::Error;
use arc_swap::ArcSwap;
use cini::{Callback, CallbackKind, Ini};
use log::{debug, warn};

use crate::credential::Credential;
use crate::formats::online;

#[derive(Debug)]
pub struct Config {
    pub motd: heapless::String<{ online::MAX_MOTD_LENGTH }>,
    pub shards: usize,
    pub buffers_per_shard: usize,
    pub max_connections_per_shard: usize,
    pub max_clients: usize,
    pub max_rooms_per_frame_rate: usize,
    pub max_spectators_per_room: usize,
    pub s3_host: Option<String>,
    pub s3_region: Option<String>,
    pub s3_access_key: Option<Credential>,
    pub s3_secret_key: Option<Credential>,
}

impl Config {
    pub fn new() -> Self {
        Self {
            motd: "It works!".try_into().unwrap(),
            shards: 0,
            buffers_per_shard: 5000,
            max_connections_per_shard: 5000,
            max_clients: 20000,
            max_rooms_per_frame_rate: 20000,
            max_spectators_per_room: 1000,
            s3_host: None,
            s3_region: None,
            s3_access_key: None,
            s3_secret_key: None,
        }
    }

    pub fn read() -> Result<Self, Error> {
        let mut config = match fs::read_to_string("run/ddd-server.conf") {
            Err(e) if e.kind() == ErrorKind::NotFound => {
                debug!("ddd-server.conf not found, using default configuration");
                Self::new()
            }
            r => r?.parse()?,
        };
        config.s3_access_key = Credential::load("run/s3-access-key.txt");
        config.s3_secret_key = Credential::load("run/s3-secret-key.txt");
        for line in format!("{config:#?}").lines() {
            debug!("{line}");
        }
        Ok(config)
    }
}

impl Default for Config {
    fn default() -> Self {
        Self::new()
    }
}

impl FromStr for Config {
    type Err = Infallible;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut config = Self::new();
        config.parse_str(s)?;
        Ok(config)
    }
}

impl Ini for Config {
    type Err = Infallible;

    fn callback(&mut self, cb: Callback) -> Result<(), Self::Err> {
        let CallbackKind::Directive(Some("Config"), key, value) = cb.kind else {
            if let CallbackKind::Section(section) = cb.kind
                && section != "Config"
            {
                warn!("Unexpected section \"{section}\"");
            }
            return Ok(());
        };

        let parse_motd = |field| parse(field, key, value, |value| Ok(value.parse()?));
        let parse_number = |field, limit| {
            parse(field, key, value, |value| {
                let value = value.parse()?;
                anyhow::ensure!(value <= limit);
                Ok(value)
            });
        };
        let parse_optional_string =
            |field| parse(field, key, value, |value| Ok(Some(value.to_owned())));

        match key {
            "motd" => parse_motd(&mut self.motd),
            "shards" => parse_number(&mut self.shards, 4096),
            "buffers_per_shard" => parse_number(&mut self.buffers_per_shard, 65536),
            "max_connections_per_shard" => parse_number(&mut self.max_connections_per_shard, 65536),
            "max_clients" => parse_number(&mut self.max_clients, 65536),
            "max_rooms_per_frame_rate" => parse_number(&mut self.max_rooms_per_frame_rate, 32768),
            "max_spectators_per_room" => parse_number(&mut self.max_spectators_per_room, 65536),
            "s3_host" => parse_optional_string(&mut self.s3_host),
            "s3_region" => parse_optional_string(&mut self.s3_region),
            _ => warn!("Unexpected key \"{key}\""),
        }
        Ok(())
    }
}

fn parse<V, P>(field: &mut V, key: &str, value: Option<&str>, parse: P)
where
    P: Fn(&str) -> Result<V, Error>,
{
    let Some(value) = value else {
        warn!("Unexpected empty value for key \"{key}\"");
        return;
    };

    match parse(value) {
        Ok(value) => *field = value,
        Err(e) => warn!("Unexpected value \"{value}\" for key \"{key}\": {e}"),
    }
}

pub type SharedConfig = Arc<ArcSwap<Config>>;
