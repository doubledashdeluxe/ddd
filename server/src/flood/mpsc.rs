use std::net::SocketAddr;
use std::sync::mpsc;
use std::thread;
use std::time::Duration;

use anyhow::Result;

use crate::buffer::Buffer;
use crate::receiver;
use crate::{BufferReceiver, BufferSender};

pub fn channel(
    senders: usize,
    buffers_per_sender: usize,
) -> (Vec<Sender>, impl receiver::Receiver) {
    let buffers = senders * buffers_per_sender;
    let (message_sender, message_receiver) = mpsc::sync_channel(buffers);
    let (senders, buffer_senders) = (0..senders)
        .map(|index| {
            let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffers_per_sender);
            for _ in 0..buffers_per_sender {
                buffer_sender.send(Buffer::new()).unwrap();
            }
            let sender = Sender { index, message_sender: message_sender.clone(), buffer_receiver };
            (sender, buffer_sender)
        })
        .collect();
    let receiver = Receiver { message_receiver, buffer_senders };
    (senders, receiver)
}

pub struct Sender {
    index: usize,
    message_sender: MessageSender,
    buffer_receiver: BufferReceiver,
}

impl Sender {
    pub fn send_to(&self, buf: &[u8], addr: SocketAddr, timeout: Duration) -> Result<usize> {
        let mut buffer = self.buffer_receiver.recv_timeout(timeout)?;
        let len = buffer.copy_from(buf);
        let message = Message { sender_index: self.index, buffer, addr };
        self.message_sender.send(message)?;
        Ok(len)
    }
}

struct Receiver {
    message_receiver: MessageReceiver,
    buffer_senders: Vec<BufferSender>,
}

impl receiver::Receiver for Receiver {
    #[expect(clippy::infinite_loop)]
    fn recv_from(&self, buf: &mut [u8]) -> Result<(usize, SocketAddr)> {
        let Ok(message) = self.message_receiver.recv() else {
            loop {
                thread::sleep(Duration::MAX);
            }
        };
        let len = message.buffer.copy_to(buf);
        let _ = self.buffer_senders[message.sender_index].send(message.buffer);
        Ok((len, message.addr))
    }
}

struct Message {
    sender_index: usize,
    buffer: Buffer,
    addr: SocketAddr,
}

type MessageSender = mpsc::SyncSender<Message>;
type MessageReceiver = mpsc::Receiver<Message>;
