use std::collections::HashMap;
use std::sync::mpsc::SyncSender;
use std::thread;
use std::time::{Duration, Instant};

use rand::SeedableRng;

use crate::clients::Clients;
use crate::crypto::ChaCha20Rng;
use crate::formats::online::FrameRate;
use crate::message::Message;
use crate::rooms::Rooms;

pub fn run(
    message_senders: &[SyncSender<Message>],
    clients: &Clients,
    rooms: &Rooms,
    frame_rate: FrameRate,
) -> ! {
    let mut next_tick = Instant::now();
    let mut client_room_ids = HashMap::new();
    let mut rng = ChaCha20Rng::from_os_rng();
    loop {
        let now = Instant::now();
        match next_tick.checked_duration_since(now) {
            Some(duration) if !duration.is_zero() => thread::sleep(duration),
            _ => {
                clients.update(now, &mut client_room_ids, rooms, &mut rng);
                rooms.update(frame_rate, &client_room_ids);
                for message_sender in message_senders {
                    let message = Message::Write { frame_rate };
                    message_sender.send(message).unwrap();
                }
                let tick_duration = match frame_rate {
                    FrameRate::SixtyHz => 16_683_333,
                    FrameRate::FiftyHz => 20_000_000,
                };
                next_tick += Duration::from_nanos(tick_duration);
            }
        }
    }
}
