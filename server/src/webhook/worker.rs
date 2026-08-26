use std::fmt::{self, Debug, Formatter};
use std::fs;
use std::sync::Arc;
use std::sync::mpsc::Receiver;

use anyhow::Result;
use arc_swap::Cache;
use log::error;
use ureq::Agent;
use ureq::http::HeaderValue;
use ureq::tls::{TlsConfig, TlsProvider};

use crate::courses::{Courses, SharedCourses};
use crate::storage;
use crate::webhook::race::Race;

pub struct Worker {
    courses: SharedCourses,
    race_receiver: Receiver<storage::Race>,
    username: String,
    avatar_url: String,
    url: String,
    buf: Vec<u8>,
    agent: Agent,
    webhook: Option<Credential>,
}

impl Worker {
    pub fn new(courses: SharedCourses, race_receiver: Receiver<storage::Race>) -> Self {
        let crypto_provider = Arc::new(rustls_graviola::default_provider());
        let agent = Agent::config_builder()
            .tls_config(
                TlsConfig::builder()
                    .provider(TlsProvider::Rustls)
                    .unversioned_rustls_crypto_provider(crypto_provider)
                    .build(),
            )
            .build()
            .new_agent();
        Self {
            courses,
            race_receiver,
            username: "dev.ddd.gg".to_owned(),
            avatar_url: "https://i.imgur.com/sTkpcT9.png".to_owned(),
            url: "https://live.dev.ddd.gg".to_owned(),
            buf: vec![],
            agent,
            webhook: Credential::load("run/webhook.txt"),
        }
    }

    pub fn run(mut self) -> ! {
        let mut courses = Cache::new(self.courses.clone());
        loop {
            let courses = courses.load();
            let mut race = self.race_receiver.recv().unwrap();
            if let Err(e) = self.send_race(courses, &mut race) {
                error!("{e}");
            }
        }
    }

    fn send_race(&mut self, courses: &Courses, race: &mut storage::Race) -> Result<()> {
        self.buf.clear();
        let race = Race::new(courses, &self.username, &self.avatar_url, &self.url, race);
        serde_json::to_writer(&mut self.buf, &race)?;

        let Some(webhook) = &self.webhook else { return Ok(()) };
        self.agent
            .post(&webhook.0)
            .content_type(HeaderValue::from_static("application/json"))
            .send(&self.buf)?;

        Ok(())
    }
}

struct Credential(String);

impl Credential {
    fn load(path: &str) -> Option<Self> {
        let mut credential = fs::read_to_string(path).ok()?;
        credential.truncate(credential.trim_end().len());
        Some(Self(credential))
    }
}

impl Debug for Credential {
    fn fmt(&self, f: &mut Formatter) -> fmt::Result {
        f.debug_tuple("Credential").finish_non_exhaustive()
    }
}
