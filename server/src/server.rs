use std::sync::mpsc;
use std::thread::Builder;

use anyhow::Result;
use noise_protocol::U8Array;

use crate::buffer::Buffer;
use crate::clients::Clients;
use crate::config::SharedConfig;
use crate::crypto::Key;
use crate::frequency::Frequency;
use crate::listener::Listener;
use crate::options::Options;
use crate::receiver::Receiver;
use crate::rooms::Rooms;
use crate::sender::Sender;
use crate::shard::Shard;
use crate::update::SharedUpdate;
use crate::updater::Updater;

pub fn spawn(
    options: Options,
    config: &SharedConfig,
    update: &SharedUpdate,
    server_k: &Key,
    shards: usize,
    senders: impl Iterator<Item = Result<impl Sender>>,
    receiver: impl Receiver,
) -> Result<()> {
    let buffers_per_shard = config.load().buffers_per_shard;
    let buffers = shards * buffers_per_shard;
    let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffers);
    for _ in 0..buffers {
        buffer_sender.send(Buffer::new())?;
    }

    let clients = Clients::new(config.load().max_clients);
    let rooms = Rooms::new(config.load().max_rooms_per_frame_rate);

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
            frequency,
        );
        Builder::new().name(format!("updater/{i}")).spawn(move || updater.run())?;
    }

    let listener = Listener::new(receiver, message_senders, buffer_receiver);
    Builder::new().name("listener".to_owned()).spawn(move || listener.run())?;

    Ok(())
}
