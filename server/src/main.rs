use std::fs::File;
use std::io::{Read, Write};

use anyhow::Result;
use noise_protocol::{DH, U8Array};

use crate::crypto::x25519::X25519;
use crate::server::Server;

mod client;
mod connection;
mod crypto;
mod formats;
mod server;

fn main() -> Result<()> {
    let server_k = match File::open("k.bin") {
        Ok(mut file) => {
            let mut server_k = <X25519 as DH>::Key::new();
            anyhow::ensure!(file.metadata()?.len() == server_k.len() as u64);
            file.read_exact(server_k.as_mut())?;
            server_k
        }
        Err(_) => {
            let server_k = X25519::genkey();
            let mut file = File::create_new("k.bin")?;
            file.write_all(server_k.as_slice())?;
            server_k
        }
    };
    let server_pk = X25519::pubkey(&server_k);
    eprint!("Server public key: ");
    for byte in server_pk {
        eprint!("{:02x?}", byte);
    }
    eprintln!();

    Server::try_new(server_k)?.run()
}
