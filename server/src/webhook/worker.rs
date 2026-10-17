use std::sync::mpsc::Receiver;
use std::time::Duration;

use anyhow::Result;
use arc_swap::Cache;
use ureq::Agent;
use ureq::http::HeaderValue;

use crate::config::{Config, SharedConfig};
use crate::courses::{Courses, SharedCourses};
use crate::http;
use crate::result_ext::ResultExt;
use crate::storage;
use crate::util;
use crate::webhook::race::Race;

pub struct Worker {
    config: SharedConfig,
    courses: SharedCourses,
    race_receiver: Receiver<storage::Race>,
    username: String,
    avatar_url: String,
    url: String,
    buf: Vec<u8>,
    agent: Agent,
}

impl Worker {
    pub fn new(
        config: SharedConfig,
        courses: SharedCourses,
        race_receiver: Receiver<storage::Race>,
    ) -> Self {
        Self {
            config,
            courses,
            race_receiver,
            username: "DDD Dev Server".to_owned(),
            avatar_url: "https://i.imgur.com/sTkpcT9.png".to_owned(),
            url: "https://dev.ddd.gg".to_owned(),
            buf: vec![],
            agent: http::agent(),
        }
    }

    pub fn run(mut self) -> ! {
        let mut config = Cache::new(self.config.clone());
        let mut courses = Cache::new(self.courses.clone());
        loop {
            let mut race = self.race_receiver.recv().unwrap();

            let config = config.load();
            let courses = courses.load();

            self.send_race(config, courses, &mut race).log_err();
        }
    }

    fn send_race(
        &mut self,
        config: &Config,
        courses: &Courses,
        race: &mut storage::Race,
    ) -> Result<()> {
        self.buf.clear();
        let race = Race::new(courses, &self.username, &self.avatar_url, &self.url, race);
        serde_json::to_writer(&mut self.buf, &race)?;

        let Some(webhook) = &config.discord_webhook else { return Ok(()) };
        util::retry(5, Duration::from_secs(1), || {
            self.agent
                .post(&webhook.0)
                .content_type(HeaderValue::from_static("application/json"))
                .send(&self.buf)
        })?;

        Ok(())
    }
}
