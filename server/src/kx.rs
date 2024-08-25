use anyhow::Result;
use noise_protocol::patterns;

use crate::blake2b::Blake2b;
use crate::chacha20poly1305::ChaCha20Poly1305;
use crate::sensitive::Sensitive;
use crate::x25519::X25519;

pub const M1_SIZE: usize = 96;
pub const M2_SIZE: usize = 48;

#[cfg(test)]
fn ik_1(client_k: Sensitive<[u8; 32]>, server_pk: [u8; 32], m1: &mut [u8]) -> ClientState {
    let pattern = patterns::noise_ik();
    let is_initiator = true;
    let prologue = "";
    let s = Some(client_k);
    let e = None;
    let rs = Some(server_pk);
    let re = None;
    let mut handshake_state = HandshakeState::new(pattern, is_initiator, prologue, s, e, rs, re);
    handshake_state.write_message(&[], m1).unwrap();
    ClientState { handshake_state }
}

pub fn ik_2(server_k: Sensitive<[u8; 32]>, m1_m2: &mut [u8]) -> Result<([u8; 32], Session)> {
    let pattern = patterns::noise_ik();
    let is_initiator = false;
    let prologue = "";
    let s = Some(server_k);
    let e = None;
    let rs = None;
    let re = None;
    let mut handshake_state = HandshakeState::new(pattern, is_initiator, prologue, s, e, rs, re);
    handshake_state.read_message(m1_m2, &mut [])?;
    handshake_state.write_message(&[], &mut m1_m2[..M2_SIZE]).unwrap();
    let client_pk = handshake_state.get_rs().unwrap();
    let (read_cipher, write_cipher) = handshake_state.get_ciphers();
    let (read_k, _) = read_cipher.extract();
    let (write_k, _) = write_cipher.extract();
    let session = Session { read_k, write_k };
    Ok((client_pk, session))
}

#[cfg(test)]
fn ik_3(client_state: ClientState, m2: &[u8]) -> Result<Session> {
    assert!(m2.len() == M1_SIZE || m2.len() == M2_SIZE);
    let mut handshake_state = client_state.handshake_state;
    handshake_state.read_message(&m2[..M2_SIZE], &mut [])?;
    let (write_cipher, read_cipher) = handshake_state.get_ciphers();
    let (read_k, _) = read_cipher.extract();
    let (write_k, _) = write_cipher.extract();
    let session = Session { read_k, write_k };
    Ok(session)
}

type HandshakeState = noise_protocol::HandshakeState<X25519, ChaCha20Poly1305, Blake2b>;

#[cfg(test)]
struct ClientState {
    handshake_state: HandshakeState,
}

pub struct Session {
    pub read_k: Sensitive<[u8; 32]>,
    pub write_k: Sensitive<[u8; 32]>,
}

#[cfg(test)]
mod tests {
    use anyhow::Result;
    use noise_protocol::{U8Array, DH};
    use orion::util;

    use crate::kx;
    use crate::x25519::X25519;

    #[test]
    fn test_invalid_server_pk() {
        let client_k = X25519::genkey();
        let server_k = X25519::genkey();
        let invalid_server_pk = X25519::pubkey(&X25519::genkey());

        let mut m1 = [0u8; kx::M1_SIZE];
        kx::ik_1(client_k, invalid_server_pk, &mut m1);
        assert!(kx::ik_2(server_k, &mut m1).is_err());
    }

    #[test]
    fn test_invalid_m1() -> Result<()> {
        let client_k = X25519::genkey();
        let server_k = X25519::genkey();
        let server_pk = X25519::pubkey(&server_k);

        let mut m1 = [0u8; kx::M1_SIZE];
        kx::ik_1(client_k, server_pk, &mut m1);
        let mut invalid_m1 = [0u8; kx::M1_SIZE];
        util::secure_rand_bytes(&mut invalid_m1)?;
        assert!(kx::ik_2(server_k, &mut invalid_m1).is_err());
        Ok(())
    }

    #[test]
    fn test_invalid_m2() -> Result<()> {
        let client_k = X25519::genkey();
        let server_k = X25519::genkey();
        let server_pk = X25519::pubkey(&server_k);

        let mut m1 = [0u8; kx::M1_SIZE];
        let client_state = kx::ik_1(client_k, server_pk, &mut m1);
        kx::ik_2(server_k, &mut m1)?;
        let mut invalid_m2 = [0u8; kx::M2_SIZE];
        util::secure_rand_bytes(&mut invalid_m2)?;
        assert!(kx::ik_3(client_state, &invalid_m2).is_err());
        Ok(())
    }

    #[test]
    fn test_valid() -> Result<()> {
        let client_k = X25519::genkey();
        let server_k = X25519::genkey();
        let server_pk = X25519::pubkey(&server_k);

        let mut m1_m2 = [0u8; kx::M1_SIZE];
        let client_state = kx::ik_1(client_k.clone(), server_pk, &mut m1_m2);
        let (client_pk, server_session) = kx::ik_2(server_k, &mut m1_m2)?;
        let client_session = kx::ik_3(client_state, &m1_m2)?;

        assert_eq!(client_pk, X25519::pubkey(&client_k));
        assert_ne!(server_session.read_k.as_slice(), server_session.write_k.as_slice());
        assert_eq!(server_session.read_k.as_slice(), client_session.write_k.as_slice());
        assert_eq!(server_session.write_k.as_slice(), client_session.read_k.as_slice());
        Ok(())
    }
}
