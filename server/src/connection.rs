use std::collections::hash_map::{Entry, HashMap};
use std::net::SocketAddr;
use std::time::{Duration, Instant};

use anyhow::{Context, Result};

use crate::client::Client;
use crate::crypto::kx;
use crate::crypto::sensitive::Sensitive;
use crate::crypto::session::Session;

pub struct Connection {
    expiration: Instant,
    addr: SocketAddr,
    client_pk: [u8; 32],
    session: Session,
    state: ConnectionState,
}

impl Connection {
    pub fn try_new(
        server_k: Sensitive<[u8; 32]>,
        now: Instant,
        addr: SocketAddr,
        message: &[u8],
    ) -> Result<Connection> {
        let expiration = now + Duration::from_secs(2);
        anyhow::ensure!(message.len() == kx::M1_SIZE);
        let mut m2 = [0u8; kx::M2_SIZE];
        let (client_pk, session) = kx::ik_2(server_k, &message, &mut m2)?;
        let state = ConnectionState::Kx { m2 };
        let connection = Connection { expiration, addr, client_pk, session, state };
        Ok(connection)
    }

    pub fn read(
        &mut self,
        now: Instant,
        message: &[u8],
        clients: &mut HashMap<[u8; 32], Client>,
    ) -> Result<()> {
        let plaintext_len = message
            .len()
            .checked_sub(Session::MAC_SIZE + Session::NONCE_SIZE)
            .context("Invalid message length")?;
        let mut plaintext = [0u8; 512];
        let plaintext = &mut plaintext[..plaintext_len];
        match self.session.decrypt(message, plaintext) {
            Ok(_) => (),
            Err(_) => return Ok(()),
        }
        let client = match self.state {
            ConnectionState::Kx { .. } => {
                // Any MITM can trivially replay M1, thus we need to wait for a valid session
                // message to consider the client to be authenticated.
                let is_full = clients.len() >= 1000;
                match clients.entry(self.client_pk) {
                    Entry::Occupied(o) => {
                        let client = o.into_mut();
                        client.set_addr(self.addr);
                        client
                    }
                    Entry::Vacant(v) if !is_full => {
                        let client = Client::new(now, self.addr);
                        v.insert(client)
                    }
                    _ => return Ok(()),
                }
            }
            ConnectionState::Session => {
                clients.get_mut(&self.client_pk).context("Disconnected client")?
            }
        };
        self.expiration = now + Duration::from_secs(30);
        self.state = ConnectionState::Session;
        client.read(now, self.addr, plaintext)
    }

    pub fn write(
        &mut self,
        now: Instant,
        message: &mut [u8],
        clients: &mut HashMap<[u8; 32], Client>,
    ) -> Result<usize> {
        anyhow::ensure!(now < self.expiration);
        match self.state {
            ConnectionState::Kx { m2 } => {
                let message = &mut message[..kx::M2_SIZE];
                message.copy_from_slice(&m2);
                Ok(kx::M2_SIZE)
            }
            ConnectionState::Session => {
                let client = clients.get_mut(&self.client_pk).context("Disconnected client")?;
                let mut plaintext = [0u8; 512];
                let plaintext_len = client.write(now, self.addr, &mut plaintext)?;
                let plaintext = &plaintext[..plaintext_len];
                let message_len = plaintext_len + Session::MAC_SIZE + Session::NONCE_SIZE;
                let message = &mut message[..message_len];
                self.session.encrypt(&plaintext, message);
                Ok(message_len)
            }
        }
    }
}

enum ConnectionState {
    Kx { m2: [u8; kx::M2_SIZE] },
    Session,
}
