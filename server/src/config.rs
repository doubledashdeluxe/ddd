use std::fs;
use std::io::ErrorKind;
use std::str::FromStr;
use std::sync::Arc;

use anyhow::{Context, Error};
use arc_swap::ArcSwap;
use cini::{Callback, CallbackKind, Ini};
use heapless::String;
use log::debug;

use crate::formats::online;

#[derive(Debug)]
pub struct Config {
    pub motd: String<{ online::MAX_MOTD_LENGTH }>,
    pub shards: usize,
    pub buffers_per_shard: usize,
    pub max_connections_per_shard: usize,
    pub max_clients: usize,
    pub max_rooms_per_frame_rate: usize,
    pub max_spectators_per_room: usize,
}

impl Config {
    pub fn new() -> Self {
        Self {
            motd: "It works!".parse().unwrap(),
            shards: 0,
            buffers_per_shard: 1000,
            max_connections_per_shard: 2000,
            max_clients: 10000,
            max_rooms_per_frame_rate: 10000,
            max_spectators_per_room: 1000,
        }
    }

    pub fn read() -> Result<Self, Error> {
        let config = match fs::read_to_string("ddd-server.conf") {
            Err(e) if e.kind() == ErrorKind::NotFound => {
                debug!("ddd-server.conf not found, using default configuration");
                Self::new()
            }
            r => r?.parse()?,
        };
        for line in format!("{config:#?}").lines() {
            debug!("{line}");
        }
        Ok(config)
    }
}

impl FromStr for Config {
    type Err = Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let mut config = Self::new();
        config.parse_str(s)?;
        Ok(config)
    }
}

impl Ini for Config {
    type Err = Error;

    fn callback(&mut self, cb: Callback) -> Result<(), Self::Err> {
        let (key, value) = match cb.kind {
            CallbackKind::Section(section) => {
                anyhow::ensure!(section == "Config");
                return Ok(());
            }
            CallbackKind::Directive(_, key, value) => (key, value),
        };

        let parse_motd = || parse(key, value, |value| Ok(value.parse()?));
        let parse_number = |limit| {
            parse(key, value, |value| {
                let value = value.parse()?;
                anyhow::ensure!(value <= limit);
                Ok(value)
            })
        };

        match key {
            "motd" => self.motd = parse_motd()?,
            "shards" => self.shards = parse_number(4096)?,
            "buffers_per_shard" => self.buffers_per_shard = parse_number(65536)?,
            "max_connections_per_shard" => self.max_connections_per_shard = parse_number(65536)?,
            "max_clients" => self.max_clients = parse_number(65536)?,
            "max_rooms_per_frame_rate" => self.max_rooms_per_frame_rate = parse_number(32768)?,
            "max_spectators_per_room" => self.max_spectators_per_room = parse_number(65536)?,
            _ => anyhow::bail!("Unexpected key \"{key}\""),
        }
        Ok(())
    }
}

fn parse<P, V>(key: &str, value: Option<&str>, parse: P) -> Result<V, Error>
where
    P: Fn(&str) -> Result<V, Error>,
{
    let Some(value) = value else {
        anyhow::bail!("Unexpected empty value for key \"{key}\"");
    };
    parse(value).with_context(|| format!("Unexpected value \"{value}\" for key \"{key}\""))
}

pub type SharedConfig = Arc<ArcSwap<Config>>;
