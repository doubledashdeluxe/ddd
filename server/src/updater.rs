use std::collections::HashMap;
use std::sync::mpsc::SyncSender;
use std::thread;
use std::time::{Duration, Instant};

use arc_swap::Cache;

use crate::clients::Clients;
use crate::config::SharedConfig;
use crate::crypto::ChaCha20Rng;
use crate::formats::online::FrameRate;
use crate::message::Message;
use crate::rooms::Rooms;

pub struct Updater {
    config: SharedConfig,
    message_senders: Vec<SyncSender<Message>>,
    clients: Clients,
    rooms: Rooms,
    frame_rate: FrameRate,
}

impl Updater {
    pub const fn new(
        config: SharedConfig,
        message_senders: Vec<SyncSender<Message>>,
        clients: Clients,
        rooms: Rooms,
        frame_rate: FrameRate,
    ) -> Self {
        Self { config, message_senders, clients, rooms, frame_rate }
    }

    pub fn run(self) -> ! {
        let mut config = Cache::new(self.config);
        let mut next_tick = Instant::now();
        let mut client_room_ids = HashMap::new();
        let mut rng: ChaCha20Rng = rand::make_rng();
        loop {
            let now = Instant::now();
            match next_tick.checked_duration_since(now) {
                Some(duration) if !duration.is_zero() => thread::sleep(duration),
                _ => {
                    let config = config.load();
                    let room_count = self.rooms.count(self.frame_rate);
                    let mut room_slots = config.max_rooms_per_frame_rate - room_count;
                    self.clients.update(
                        now,
                        config,
                        self.frame_rate,
                        &mut client_room_ids,
                        &self.rooms,
                        &mut room_slots,
                        &mut rng,
                    );
                    self.rooms.update(self.frame_rate, &client_room_ids);
                    let client_count = self.clients.count();
                    let client_slots = config.max_clients - client_count;
                    let client_slots = client_slots / self.message_senders.len();
                    for message_sender in &self.message_senders {
                        let message = Message::Write { frame_rate: self.frame_rate, client_slots };
                        message_sender.send(message).unwrap();
                    }
                    let tick_duration = match self.frame_rate {
                        FrameRate::SixtyHz => 16_683_333,
                        FrameRate::FiftyHz => 20_000_000,
                    };
                    next_tick += Duration::from_nanos(tick_duration);
                }
            }
        }
    }
}
