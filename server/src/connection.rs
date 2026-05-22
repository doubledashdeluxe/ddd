use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::{Context, Result};

use crate::clients::Clients;
use crate::config::Config;
use crate::crypto::kx;
use crate::crypto::session::Session;
use crate::crypto::{Key, PublicKey};
use crate::formats::online::FrameRate;
use crate::rooms::Rooms;

pub struct Connection {
    expiration: Instant,
    addr: SocketAddr,
    client_pk: PublicKey,
    session: Session,
    state: State,
}

impl Connection {
    pub fn new(server_k: Key, now: Instant, addr: SocketAddr, message: &[u8]) -> Result<Self> {
        let expiration = now + Duration::from_secs(2);
        anyhow::ensure!(message.len() == kx::M1_SIZE);
        let mut m2 = [0u8; kx::M2_SIZE];
        let (client_pk, session) = kx::ik_2(server_k, message, &mut m2)?;
        let state = State::Kx { m2 };
        let connection = Self { expiration, addr, client_pk, session, state };
        Ok(connection)
    }

    pub fn read(
        &mut self,
        now: Instant,
        message: &[u8],
        clients: &Clients,
        client_slots: &mut usize,
    ) -> Result<()> {
        let plaintext_len = message
            .len()
            .checked_sub(Session::MAC_SIZE + Session::NONCE_SIZE)
            .context("Invalid message length")?;
        let mut plaintext = [0u8; 512];
        let plaintext = &mut plaintext[..plaintext_len];
        match self.session.decrypt(message, plaintext) {
            Ok(()) => (),
            Err(_) => return Ok(()),
        }
        let mut client = match self.state {
            State::Kx { .. } => {
                // Any MITM can trivially replay M1, thus we need to wait for a valid session
                // message to consider the client to be authenticated.
                clients.insert(now, client_slots, self.addr, self.client_pk)
            }
            State::Session => clients.get(&self.client_pk),
        }?;
        self.expiration = now + Duration::from_secs(30);
        self.state = State::Session;
        client.read(now, self.addr, plaintext)
    }

    pub fn write(
        &mut self,
        now: Instant,
        config: &Config,
        frame_rate: FrameRate,
        message: &mut [u8],
        clients: &Clients,
        player_count: usize,
        rooms: &Rooms,
    ) -> Result<Option<usize>> {
        anyhow::ensure!(now < self.expiration);
        match self.state {
            State::Kx { m2 } => match frame_rate {
                FrameRate::SixtyHz => {
                    let message = &mut message[..kx::M2_SIZE];
                    message.copy_from_slice(&m2);
                    Ok(Some(kx::M2_SIZE))
                }
                FrameRate::FiftyHz => Ok(None),
            },
            State::Session => {
                let mut plaintext = [0u8; 512];
                let plaintext_len = clients.read(&self.client_pk, |client| {
                    client.write(frame_rate, self.addr, &mut plaintext, config, player_count, rooms)
                })??;
                let Some(plaintext_len) = plaintext_len else {
                    return Ok(None);
                };
                let plaintext = &plaintext[..plaintext_len];
                let message_len = plaintext_len + Session::MAC_SIZE + Session::NONCE_SIZE;
                let message = &mut message[..message_len];
                self.session.encrypt(plaintext, message);
                Ok(Some(message_len))
            }
        }
    }
}

enum State {
    Kx { m2: [u8; kx::M2_SIZE] },
    Session,
}
