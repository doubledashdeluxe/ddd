extern "C" {
#include <monocypher/monocypher.h>
}
#include <payload/crypto/KX.hh>
#include <payload/crypto/Random.hh>
#include <snitch/snitch_all.hpp>

#include <array>

TEST_CASE("Invalid server PK") {
    Array<u8, 32> clientK, clientEphemeralK, serverK, serverEphemeralK, invalidServerK,
            invalidServerPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    Random::Get(serverEphemeralK.values(), serverEphemeralK.count());
    Random::Get(invalidServerK.values(), invalidServerK.count());
    crypto_x25519_public_key(invalidServerPK.values(), invalidServerK.values());

    KX::ClientState clientState(clientK, clientEphemeralK, invalidServerPK);
    std::array<u8, KX::M1Size> m1;
    while (!clientState.getM1(m1.data())) {
        CHECK(clientState.update());
    }
    KX::ServerState serverState(serverK, serverEphemeralK);
    CHECK(serverState.setM1(m1.data()));
    while (serverState.update()) {}
    CHECK_FALSE(serverState.hasM2());
    CHECK_FALSE(serverState.serverSession());
}

TEST_CASE("Invalid m1") {
    Array<u8, 32> clientK, clientEphemeralK, serverK, serverEphemeralK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    Random::Get(serverEphemeralK.values(), serverEphemeralK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    KX::ClientState clientState(clientK, clientEphemeralK, serverPK);
    while (!clientState.hasM1()) {
        CHECK(clientState.update());
    }
    KX::ServerState serverState(serverK, serverEphemeralK);
    std::array<u8, KX::M1Size> invalidM1;
    Random::Get(invalidM1.data(), invalidM1.size());
    CHECK(serverState.setM1(invalidM1.data()));
    while (serverState.update()) {}
    CHECK_FALSE(serverState.hasM2());
    CHECK_FALSE(serverState.serverSession());
}

TEST_CASE("Invalid m2") {
    Array<u8, 32> clientK, clientEphemeralK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    KX::ClientState clientState(clientK, clientEphemeralK, serverPK);
    while (!clientState.hasM1()) {
        CHECK(clientState.update());
    }
    std::array<u8, KX::M2Size> invalidM2;
    Random::Get(invalidM2.data(), invalidM2.size());
    CHECK(clientState.setM2(invalidM2.data()));
    while (clientState.update()) {}
    CHECK_FALSE(clientState.clientSession());
}

TEST_CASE("Valid") {
    Array<u8, 32> clientK, clientEphemeralK, serverK, serverEphemeralK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    Random::Get(serverEphemeralK.values(), serverEphemeralK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    KX::ClientState clientState(clientK, clientEphemeralK, serverPK);
    std::array<u8, KX::M1Size> m1;
    while (!clientState.getM1(m1.data())) {
        CHECK(clientState.update());
    }
    KX::ServerState serverState(serverK, serverEphemeralK);
    CHECK(serverState.setM1(m1.data()));
    while (serverState.update()) {}
    std::array<u8, KX::M2Size> m2;
    CHECK(serverState.getM2(m2.data()));
    const Array<u8, 32> *clientPK = serverState.clientPK();
    CHECK(clientPK);
    const Session *serverSession = serverState.serverSession();
    CHECK(serverSession);
    CHECK(clientState.setM2(m2.data()));
    while (clientState.update()) {}
    const Session *clientSession = clientState.clientSession();
    CHECK(clientSession);

    Array<u8, 32> expectedClientPK;
    crypto_x25519_public_key(expectedClientPK.values(), clientK.values());
    CHECK(*clientPK == expectedClientPK);
    CHECK(serverSession->m_readK != serverSession->m_writeK);
    CHECK(serverSession->m_readK == clientSession->m_writeK);
    CHECK(serverSession->m_writeK == clientSession->m_readK);
}
