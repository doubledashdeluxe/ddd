use std::collections::HashMap;
use std::sync::Arc;
use std::thread;
use std::time::Instant;

use arc_swap::Cache;

use crate::MessageSender;
use crate::clients::Clients;
use crate::config::SharedConfig;
use crate::crypto::ChaCha20Rng;
use crate::frequency::Frequency;
use crate::message::Message;
use crate::rooms::Rooms;
use crate::storage::Storage;

pub struct Updater {
    config: SharedConfig,
    message_senders: Vec<MessageSender>,
    clients: Arc<Clients>,
    rooms: Arc<Rooms>,
    storage: Arc<Storage>,
    frequency: Frequency,
}

impl Updater {
    pub const fn new(
        config: SharedConfig,
        message_senders: Vec<MessageSender>,
        clients: Arc<Clients>,
        rooms: Arc<Rooms>,
        storage: Arc<Storage>,
        frequency: Frequency,
    ) -> Self {
        Self { config, message_senders, clients, rooms, storage, frequency }
    }

    pub fn run(self) -> ! {
        let mut config = Cache::new(self.config);
        let mut next_tick = Instant::now();
        let mut client_room_ids = HashMap::new();
        let mut rng: ChaCha20Rng = rand::make_rng();
        loop {
            let now = Instant::now();
            if let Some(duration) = next_tick.checked_duration_since(now)
                && !duration.is_zero()
            {
                thread::sleep(duration);
                continue;
            }

            let config = config.load();
            let frame_rate = self.frequency.try_into().ok();
            let mut room_slots = frame_rate.map_or(0, |frame_rate| {
                let room_count = self.rooms.count(frame_rate);
                config.max_rooms_per_frame_rate - room_count
            });
            self.clients.update(
                now,
                config,
                self.frequency,
                &mut client_room_ids,
                &self.rooms,
                &mut room_slots,
                &self.storage,
                &mut rng,
            );
            if let Some(frame_rate) = frame_rate {
                self.rooms.update(frame_rate, &client_room_ids, &self.storage);
            }
            let client_count = self.clients.count();
            let client_slots = config.max_clients - client_count;
            let client_slots = client_slots / self.message_senders.len();
            for message_sender in &self.message_senders {
                let message = Message::Write { frequency: self.frequency, client_slots };
                message_sender.send(message).unwrap();
            }
            next_tick += self.frequency.period();
        }
    }
}
