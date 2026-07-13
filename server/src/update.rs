use std::fs;
use std::io::{self, ErrorKind};
use std::sync::Arc;

use anyhow::Result;
use arc_swap::ArcSwap;
use heapless::Vec;
use log::debug;
use rawzip::{CompressionMethod, ZipArchive};
use yazi::Format;

use crate::formats::online::{MAX_UPDATE_CHANGELOG_LENGTH, UPDATE_CHUNK_SIZE};

pub struct Update {
    pub update: Box<[u8]>,
    pub changelog: Vec<u8, MAX_UPDATE_CHANGELOG_LENGTH>,
}

impl Update {
    pub fn read() -> Result<Option<Self>> {
        let update = match fs::read("build/update.zip") {
            Err(e) if e.kind() == ErrorKind::NotFound => {
                debug!("Update not found");
                return Ok(None);
            }
            r => r?,
        };
        u16::try_from(update.len().div_ceil(UPDATE_CHUNK_SIZE))?;
        let archive = ZipArchive::from_slice(&update)?;
        let entry = archive.entries().find_map(|entry| {
            let entry = match entry {
                Ok(entry) => entry,
                Err(e) => return Some(Err(e)),
            };
            let path = entry.file_path();
            (path.as_bytes() == b"changelog.txt").then_some(Ok(entry))
        });
        let changelog = if let Some(entry) = entry {
            let entry = entry?;
            anyhow::ensure!(entry.compression_method() == CompressionMethod::DEFLATE);
            let wayfinder = entry.wayfinder();
            let entry = archive.get_entry(wayfinder)?;
            let (changelog, _) = yazi::decompress(entry.data(), Format::Raw)
                .map_err(|e| anyhow::anyhow!("{e:?}"))?;
            let mut reader = entry.verifying_reader(&*changelog);
            io::copy(&mut reader, &mut io::empty())?;
            (*changelog).try_into()?
        } else {
            Vec::new()
        };
        debug!("Found update with size {}", update.len());
        Ok(Some(Self { update: update.into(), changelog }))
    }
}

pub type SharedUpdate = Arc<ArcSwap<Option<Update>>>;
