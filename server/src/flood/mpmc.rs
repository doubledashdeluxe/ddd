use std::iter;
use std::net::SocketAddr;
use std::sync::mpsc;

use anyhow::Result;

use crate::buffer::Buffer;
use crate::sender;
use crate::{BufferReceiver, BufferSender};

pub fn channel(pairs: usize, buffers_per_pair: usize) -> (Vec<impl sender::Sender>, Vec<Receiver>) {
    iter::repeat_with(|| {
        let (message_sender, message_receiver) = mpsc::sync_channel(buffers_per_pair);
        let (buffer_sender, buffer_receiver) = mpsc::sync_channel(buffers_per_pair);
        for _ in 0..buffers_per_pair {
            buffer_sender.send(Buffer::new()).unwrap();
        }
        let sender = Sender { message_sender, buffer_receiver };
        let receiver = Receiver { message_receiver, buffer_sender };
        (sender, receiver)
    })
    .take(pairs)
    .collect()
}

struct Sender {
    message_sender: MessageSender,
    buffer_receiver: BufferReceiver,
}

impl sender::Sender for Sender {
    fn send_to(&self, buf: &[u8], addr: SocketAddr) -> Result<usize> {
        let Ok(mut buffer) = self.buffer_receiver.try_recv() else { return Ok(0) };
        let len = buffer.copy_from(buf);
        let message = Message { buffer, addr };
        let _ = self.message_sender.send(message);
        Ok(len)
    }
}

pub struct Receiver {
    message_receiver: MessageReceiver,
    buffer_sender: BufferSender,
}

impl Receiver {
    pub fn recv_from(&self, buf: &mut [u8]) -> Result<(usize, SocketAddr)> {
        let message = self.message_receiver.try_recv()?;
        let len = message.buffer.copy_to(buf);
        self.buffer_sender.send(message.buffer)?;
        Ok((len, message.addr))
    }
}

struct Message {
    buffer: Buffer,
    addr: SocketAddr,
}

type MessageSender = mpsc::SyncSender<Message>;
type MessageReceiver = mpsc::Receiver<Message>;
