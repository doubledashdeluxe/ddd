use std::hash::{BuildHasher, RandomState};
use std::net::UdpSocket;
use std::sync::mpsc::{Receiver, SyncSender, TrySendError};

use log::error;

use crate::buffer::Buffer;
use crate::message::Message;

pub struct Listener {
    socket: UdpSocket,
    message_senders: Vec<SyncSender<Message>>,
    buffer_receiver: Receiver<Buffer>,
}

impl Listener {
    pub const fn new(
        socket: UdpSocket,
        message_senders: Vec<SyncSender<Message>>,
        buffer_receiver: Receiver<Buffer>,
    ) -> Self {
        Self { socket, message_senders, buffer_receiver }
    }

    pub fn run(&self) -> ! {
        let random_state = RandomState::new();
        loop {
            let mut buffer = self.buffer_receiver.recv().unwrap();
            buffer.reset_len();
            let (len, addr) = loop {
                match self.socket.recv_from(buffer.as_mut_slice()) {
                    Ok(r) => break r,
                    Err(e) => {
                        error!("{e}");
                    }
                }
            };
            buffer.set_len(len);
            let index = random_state.hash_one(addr) as usize % self.message_senders.len();
            let message = Message::Read { buffer, addr };
            match self.message_senders[index].try_send(message) {
                Err(TrySendError::Full(_)) => (),
                r => r.unwrap(),
            }
        }
    }
}
