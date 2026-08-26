use std::path::Path;
use std::sync::{Arc, mpsc};
use std::thread::Builder;

use anyhow::Result;
use log::trace;
use noise_protocol::U8Array;

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::config::SharedConfig;
use crate::courses::SharedCourses;
use crate::crypto::Key;
use crate::frequency::Frequency;
use crate::listener::Listener;
use crate::options::Options;
use crate::receiver::Receiver;
use crate::rooms::Rooms;
use crate::sender::Sender;
use crate::shard::Shard;
use crate::storage::{Storage, Worker as StorageWorker};
use crate::update::SharedUpdate;
use crate::updater::Updater;
use crate::webhook::Worker as WebhookWorker;

pub fn spawn(
    options: Options,
    config: &SharedConfig,
    update: &SharedUpdate,
    courses: &SharedCourses,
    server_k: &Key,
    shards: usize,
    senders: impl Iterator<Item = Result<impl Sender>>,
    receiver: impl Receiver,
    storage_path: impl AsRef<Path>,
) -> Result<()> {
    let buffers_per_shard = config.load().buffers_per_shard;
    let buffers = shards * buffers_per_shard;
    let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffers);
    for _ in 0..buffers {
        buffer_sender.send(Buffer::new())?;
    }

    let clients = Arc::new(Clients::new(config.load().max_clients));
    let rooms = Arc::new(Rooms::new(config.load().max_rooms_per_frame_rate));

    let (batch_sender, batch_receiver) =
        mpsc::sync_channel(2 * config.load().max_rooms_per_frame_rate);
    let (storage, storage_init) = Storage::load(batch_sender, storage_path)?;
    let storage = Arc::new(storage);
    trace!("Next player number: {}", storage_init.player_number);
    trace!("Next room number: {}", storage_init.room_number);
    trace!("Next race number: {}", storage_init.race_number);

    let (webhook_race_sender, webhook_race_receiver) =
        mpsc::sync_channel(2 * config.load().max_rooms_per_frame_rate);
    let webhook_worker = WebhookWorker::new(courses.clone(), webhook_race_receiver);
    Builder::new().name("webhook".to_owned()).spawn(|| webhook_worker.run())?;

    let storage_worker = StorageWorker::new(batch_receiver, storage_init, webhook_race_sender);
    Builder::new().name("storage".to_owned()).spawn(move || storage_worker.run())?;

    let message_senders: Result<_> = senders
        .enumerate()
        .map(|(i, sender)| {
            let (message_sender, message_receiver) = mpsc::sync_channel(buffers_per_shard);
            let shard = Shard::new(
                options.net_sim,
                config.clone(),
                update.clone(),
                server_k.clone(),
                sender?,
                message_receiver,
                buffer_sender.clone(),
                clients.clone(),
                rooms.clone(),
            );
            Builder::new().name(format!("shard/{i}")).spawn(|| shard.run())?;
            Ok(message_sender)
        })
        .collect();
    let message_senders: Vec<_> = message_senders?;
    assert_eq!(message_senders.len(), shards);

    let frequencies = [Frequency::SixtyHz, Frequency::FiftyHz, Frequency::FiveHundredHz];
    for (i, frequency) in frequencies.into_iter().enumerate() {
        let updater = Updater::new(
            config.clone(),
            message_senders.clone(),
            clients.clone(),
            rooms.clone(),
            storage.clone(),
            frequency,
        );
        Builder::new().name(format!("updater/{i}")).spawn(move || updater.run())?;
    }

    let listener = Listener::new(receiver, message_senders, buffer_receiver);
    Builder::new().name("listener".to_owned()).spawn(move || listener.run())?;

    Ok(())
}
