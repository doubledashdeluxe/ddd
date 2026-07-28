use std::net::SocketAddr;
use std::time::{Duration, Instant};

use crate::crypto::PublicKey;
use crate::flood::counters::Counters;
use crate::flood::mpmc::Receiver;
use crate::flood::mpsc::Sender;
use crate::formats::online::BUFFER_SIZE;

pub trait Flood {
    fn new(server_pk: PublicKey, thread_index: usize) -> Self;
    fn read(&mut self, message: &[u8], addr: SocketAddr);
    fn write(&mut self, message: &mut [u8]) -> (usize, SocketAddr);
    fn ops(&self) -> u64;

    fn flood(
        sender: &Sender,
        receiver: &Receiver,
        server_pk: PublicKey,
        duration: Duration,
        thread_index: usize,
    ) -> Counters
    where
        Self: Sized,
    {
        let mut counters = Counters::new();
        let mut flood = Self::new(server_pk, thread_index);
        let mut buffer = [0u8; BUFFER_SIZE as usize];

        let instant = Instant::now();
        while instant.elapsed() < duration {
            for _ in 0..64 {
                let (message_len, addr) = flood.write(&mut buffer);
                let message = &buffer[..message_len];
                let Some(timeout) = duration.checked_sub(instant.elapsed()) else { break };
                let Ok(_) = sender.send_to(message, addr, timeout) else { continue };
                counters.tx += 1;
            }

            for _ in 0..64 {
                let Ok((message_len, addr)) = receiver.recv_from(&mut buffer) else { break };
                let message = &buffer[..message_len];
                flood.read(message, addr);
                counters.rx += 1;
            }
        }

        counters.ops = flood.ops();
        counters
    }
}
