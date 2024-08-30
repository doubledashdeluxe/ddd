// clang-format off
//
// Based on https://github.com/blckngm/noise-rust/blob/master/noise-protocol/src/handshakestate.rs
//
// clang-format on

#include "KX.hh"

#include "payload/payload/crypto/Random.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <string.h>
}

KX::ClientState::ClientState() {}

KX::ClientState::~ClientState() {
    crypto_wipe(m_ephemeralK.values(), m_ephemeralK.count());
    crypto_wipe(m_k.values(), m_k.count());
}

void KX::IK1(const Array<u8, 32> &clientK, const Array<u8, 32> &serverPK, u8 m1[M1Size],
        ClientState &clientState) {
    clientState = ClientState();
    clientState.m_k = clientK;
    SymmetricState &symmetricState = clientState.m_symmetricState;

    // Prologue
    symmetricState.mixHash(nullptr, 0);

    // S
    symmetricState.mixHash(serverPK.values(), serverPK.count());

    // E
    Random::Get(clientState.m_ephemeralK.values(), clientState.m_ephemeralK.count());
    crypto_x25519_public_key(m1 + 0, clientState.m_ephemeralK.values());
    symmetricState.mixHash(m1 + 0, 32);

    // ES
    symmetricState.mixDH(clientState.m_ephemeralK, serverPK);

    // S
    Array<u8, 32> clientPK;
    crypto_x25519_public_key(clientPK.values(), clientK.values());
    symmetricState.encryptAndHash(clientPK.values(), clientPK.count(), m1 + 32);

    // SS
    symmetricState.mixDH(clientK, serverPK);

    // M1 finalization
    symmetricState.encryptAndHash(nullptr, 0, m1 + 80);
}

bool KX::IK2(const Array<u8, 32> &serverK, const u8 m1[M1Size], u8 m2[M2Size],
        Array<u8, 32> &clientPK, Session &serverSession) {
    SymmetricState symmetricState;

    // Prologue
    symmetricState.mixHash(nullptr, 0);

    // S
    Array<u8, 32> serverPK;
    crypto_x25519_public_key(serverPK.values(), serverK.values());
    symmetricState.mixHash(serverPK.values(), serverPK.count());

    // E
    Array<u8, 32> clientEphemeralPK;
    memcpy(clientEphemeralPK.values(), m1 + 0, clientEphemeralPK.count());
    symmetricState.mixHash(clientEphemeralPK.values(), clientEphemeralPK.count());

    // ES
    symmetricState.mixDH(serverK, clientEphemeralPK);

    // S
    if (!symmetricState.decryptAndHash(m1 + 32, clientPK.values(), clientPK.count())) {
        return false;
    }

    // SS
    symmetricState.mixDH(serverK, clientPK);

    // M1 finalization
    if (!symmetricState.decryptAndHash(m1 + 80, nullptr, 0)) {
        return false;
    }

    // E
    Array<u8, 32> serverEphemeralK;
    Random::Get(serverEphemeralK.values(), serverEphemeralK.count());
    crypto_x25519_public_key(m2 + 0, serverEphemeralK.values());
    symmetricState.mixHash(m2 + 0, 32);

    // EE
    symmetricState.mixDH(serverEphemeralK, clientEphemeralPK);

    // SE
    symmetricState.mixDH(serverEphemeralK, clientPK);
    crypto_wipe(serverEphemeralK.values(), serverEphemeralK.count());

    // M2 finalization
    symmetricState.encryptAndHash(nullptr, 0, m2 + 32);

    serverSession.reset();
    symmetricState.computeSessionKeys(serverSession.m_readK, serverSession.m_writeK);
    return true;
}

bool KX::IK3(ClientState &clientState, const u8 m2[M2Size], Session &clientSession) {
    SymmetricState &symmetricState = clientState.m_symmetricState;

    // E
    Array<u8, 32> serverEphemeralPK;
    memcpy(serverEphemeralPK.values(), m2 + 0, serverEphemeralPK.count());
    symmetricState.mixHash(serverEphemeralPK.values(), serverEphemeralPK.count());

    // EE
    symmetricState.mixDH(clientState.m_ephemeralK, serverEphemeralPK);

    // SE
    symmetricState.mixDH(clientState.m_k, serverEphemeralPK);

    // M2 finalization
    if (!symmetricState.decryptAndHash(m2 + 32, nullptr, 0)) {
        return false;
    }

    clientSession.reset();
    symmetricState.computeSessionKeys(clientSession.m_writeK, clientSession.m_readK);
    clientState = ClientState();
    return true;
}
