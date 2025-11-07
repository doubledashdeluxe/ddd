use std::sync::mpsc::SyncSender;
use std::thread;
use std::time::{Duration, Instant};

use anyhow::Result;

use crate::clients::Clients;
use crate::message::Message;
use crate::rooms::Rooms;

pub fn run(
    message_senders: Vec<SyncSender<Message>>,
    clients: Clients,
    rooms: Rooms,
) -> Result<()> {
    let mut next_tick = Instant::now();
    loop {
        let now = Instant::now();
        match next_tick.checked_duration_since(now) {
            Some(duration) if !duration.is_zero() => thread::sleep(duration),
            _ => {
                clients.update(now, &rooms);
                rooms.update(&clients);
                for message_sender in &message_senders {
                    let message = Message::Write;
                    message_sender.send(message)?;
                }
                next_tick += Duration::from_nanos(16_683_333);
            }
        }
    }
}
