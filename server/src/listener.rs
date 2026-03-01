use std::hash::{BuildHasher, RandomState};
use std::net::UdpSocket;
use std::sync::mpsc::{Receiver, SyncSender};

use anyhow::Result;

use crate::buffer::Buffer;
use crate::message::Message;

pub fn run(
    socket: &UdpSocket,
    message_senders: &[SyncSender<Message>],
    buffer_receiver: &Receiver<Buffer>,
) -> Result<()> {
    let random_state = RandomState::new();
    loop {
        let mut buffer = buffer_receiver.recv()?;
        buffer.reset_len();
        let (len, addr) = socket.recv_from(buffer.as_mut_slice())?;
        buffer.set_len(len);
        let index = random_state.hash_one(addr) as usize % message_senders.len();
        let message = Message::Read { addr, buffer };
        message_senders[index].send(message)?;
    }
}
