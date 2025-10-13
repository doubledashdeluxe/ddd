use anyhow::Result;
use noise_protocol::patterns;

use crate::crypto::blake2b::Blake2b;
use crate::crypto::chacha20poly1305::ChaCha20Poly1305;
use crate::crypto::sensitive::Sensitive;
use crate::crypto::session::Session;
use crate::crypto::x25519::X25519;

pub const M1_SIZE: usize = 96;
pub const M2_SIZE: usize = 48;

#[cfg(test)]
fn ik_1(client_k: Sensitive<[u8; 32]>, server_pk: [u8; 32], m1: &mut [u8]) -> ClientState {
    assert!(m1.len() == M1_SIZE);
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

pub fn ik_2(
    server_k: Sensitive<[u8; 32]>,
    m1: &[u8],
    m2: &mut [u8],
) -> Result<([u8; 32], Session)> {
    assert!(m1.len() == M1_SIZE);
    assert!(m2.len() == M2_SIZE);
    let pattern = patterns::noise_ik();
    let is_initiator = false;
    let prologue = "";
    let s = Some(server_k);
    let e = None;
    let rs = None;
    let re = None;
    let mut handshake_state = HandshakeState::new(pattern, is_initiator, prologue, s, e, rs, re);
    handshake_state.read_message(m1, &mut [])?;
    handshake_state.write_message(&[], m2).unwrap();
    let client_pk = handshake_state.get_rs().unwrap();
    let (read_cipher, write_cipher) = handshake_state.get_ciphers();
    let (read_k, _) = read_cipher.extract();
    let (write_k, _) = write_cipher.extract();
    let session = Session::new(read_k, write_k);
    Ok((client_pk, session))
}

#[cfg(test)]
fn ik_3(client_state: ClientState, m2: &[u8]) -> Result<Session> {
    assert!(m2.len() == M2_SIZE);
    let mut handshake_state = client_state.handshake_state;
    handshake_state.read_message(&m2, &mut [])?;
    let (write_cipher, read_cipher) = handshake_state.get_ciphers();
    let (read_k, _) = read_cipher.extract();
    let (write_k, _) = write_cipher.extract();
    let session = Session::new(read_k, write_k);
    Ok(session)
}

type HandshakeState = noise_protocol::HandshakeState<X25519, ChaCha20Poly1305, Blake2b>;

#[cfg(test)]
struct ClientState {
    handshake_state: HandshakeState,
}

#[cfg(test)]
mod tests {
    use anyhow::Result;
    use noise_protocol::{DH, U8Array};
    use orion::util;

    use crate::crypto::kx;
    use crate::crypto::x25519::X25519;

    #[test]
    fn test_invalid_server_pk() {
        let client_k = X25519::genkey();
        let server_k = X25519::genkey();
        let invalid_server_pk = X25519::pubkey(&X25519::genkey());

        let mut m1 = [0u8; kx::M1_SIZE];
        kx::ik_1(client_k, invalid_server_pk, &mut m1);
        let mut m2 = [0u8; kx::M2_SIZE];
        assert!(kx::ik_2(server_k, &m1, &mut m2).is_err());
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
        let mut m2 = [0u8; kx::M2_SIZE];
        assert!(kx::ik_2(server_k, &invalid_m1, &mut m2).is_err());
        Ok(())
    }

    #[test]
    fn test_invalid_m2() -> Result<()> {
        let client_k = X25519::genkey();
        let server_k = X25519::genkey();
        let server_pk = X25519::pubkey(&server_k);

        let mut m1 = [0u8; kx::M1_SIZE];
        let client_state = kx::ik_1(client_k, server_pk, &mut m1);
        let mut m2 = [0u8; kx::M2_SIZE];
        kx::ik_2(server_k, &m1, &mut m2)?;
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

        let mut m1 = [0u8; kx::M1_SIZE];
        let client_state = kx::ik_1(client_k.clone(), server_pk, &mut m1);
        let mut m2 = [0u8; kx::M2_SIZE];
        let (client_pk, server_session) = kx::ik_2(server_k, &m1, &mut m2)?;
        let client_session = kx::ik_3(client_state, &m2)?;

        assert_eq!(client_pk, X25519::pubkey(&client_k));
        assert_ne!(server_session.read_k().as_slice(), server_session.write_k().as_slice());
        assert_eq!(server_session.read_k().as_slice(), client_session.write_k().as_slice());
        assert_eq!(server_session.write_k().as_slice(), client_session.read_k().as_slice());
        Ok(())
    }
}
