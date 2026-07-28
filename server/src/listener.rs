use std::hash::{BuildHasher, RandomState};

use log::error;

use crate::message::Message;
use crate::receiver::Receiver;
use crate::{BufferReceiver, MessageSender};

pub struct Listener<R: Receiver> {
    receiver: R,
    message_senders: Vec<MessageSender>,
    buffer_receiver: BufferReceiver,
}

impl<R: Receiver> Listener<R> {
    pub const fn new(
        receiver: R,
        message_senders: Vec<MessageSender>,
        buffer_receiver: BufferReceiver,
    ) -> Self {
        Self { receiver, message_senders, buffer_receiver }
    }

    pub fn run(self) -> ! {
        let random_state = RandomState::new();
        loop {
            // It's fine to unwrap because this can only fail if all shards have crashed.
            let mut buffer = self.buffer_receiver.recv().unwrap();
            buffer.reset_len();
            let (len, addr) = loop {
                match self.receiver.recv_from(buffer.as_mut_slice()) {
                    Ok(r) => break r,
                    Err(e) => {
                        error!("{e}");
                    }
                }
            };
            buffer.set_len(len);
            let index = random_state.hash_one(addr) as usize % self.message_senders.len();
            let message = Message::Read { buffer, addr };
            // It's fine to unwrap because this can only fail if the shard has crashed.
            // We don't want `try_send` because we cannot recover dropped buffers.
            // The downside is that the listener can be bottlenecked by the slowest shard, but only
            // once its queue is full.
            self.message_senders[index].send(message).unwrap();
        }
    }
}
