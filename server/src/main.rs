use std::fs::File;
use std::io::{Read, Write};
use std::net::UdpSocket;

use anyhow::Result;
use noise_protocol::{U8Array, DH};

use crate::x25519::X25519;

mod blake2b;
mod chacha20poly1305;
mod kx;
mod sensitive;
mod x25519;

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

    let socket = UdpSocket::bind("0.0.0.0:3549")?;
    loop {
        let mut m1_m2 = [0u8; kx::M1_SIZE];
        let (read_size, addr) = socket.recv_from(&mut m1_m2)?;
        if read_size != kx::M1_SIZE {
            continue;
        }
        let Ok((client_pk, session)) = kx::ik_2(server_k.clone(), &mut m1_m2) else { continue };
        socket.send_to(&mut m1_m2[..kx::M2_SIZE], addr)?;
        eprintln!("--");
        eprint!("Client public key: ");
        for byte in client_pk {
            eprint!("{:02x?}", byte);
        }
        eprintln!();
        // TODO don't print this (duh)
        eprint!("Read key: ");
        for byte in session.read_k.as_slice() {
            eprint!("{:02x?}", byte);
        }
        eprintln!();
        eprint!("Write key: ");
        for byte in session.write_k.as_slice() {
            eprint!("{:02x?}", byte);
        }
        eprintln!();
    }
}
