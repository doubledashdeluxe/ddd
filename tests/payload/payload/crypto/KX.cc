#include <lest.hpp>
extern "C" {
#include <monocypher/monocypher.h>
}
#include <payload/crypto/KX.hh>
#include <payload/crypto/Random.hh>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("Invalid server PK") {
    Array<u8, 32> clientK, serverK, invalidServerK, invalidServerPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    Random::Get(invalidServerK.values(), invalidServerK.count());
    crypto_x25519_public_key(invalidServerPK.values(), invalidServerK.values());

    KX::ClientState clientState(clientK, invalidServerPK);
    std::array<u8, KX::M1Size> m1;
    while (!clientState.getM1(m1.data())) {
        EXPECT(clientState.update());
    }
    KX::ServerState serverState(serverK);
    EXPECT(serverState.setM1(m1.data()));
    while (serverState.update()) {}
    EXPECT_NOT(serverState.hasM2());
    EXPECT_NOT(serverState.serverSession());
}

CASE("Invalid m1") {
    Array<u8, 32> clientK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    KX::ClientState clientState(clientK, serverPK);
    while (!clientState.hasM1()) {
        EXPECT(clientState.update());
    }
    KX::ServerState serverState(serverK);
    std::array<u8, KX::M1Size> invalidM1;
    Random::Get(invalidM1.data(), invalidM1.size());
    EXPECT(serverState.setM1(invalidM1.data()));
    while (serverState.update()) {}
    EXPECT_NOT(serverState.hasM2());
    EXPECT_NOT(serverState.serverSession());
}

CASE("Invalid m2") {
    Array<u8, 32> clientK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    KX::ClientState clientState(clientK, serverPK);
    while (!clientState.hasM1()) {
        EXPECT(clientState.update());
    }
    std::array<u8, KX::M2Size> invalidM2;
    Random::Get(invalidM2.data(), invalidM2.size());
    EXPECT(clientState.setM2(invalidM2.data()));
    while (clientState.update()) {}
    EXPECT_NOT(clientState.clientSession());
}

CASE("Valid") {
    Array<u8, 32> clientK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    KX::ClientState clientState(clientK, serverPK);
    std::array<u8, KX::M1Size> m1;
    while (!clientState.getM1(m1.data())) {
        EXPECT(clientState.update());
    }
    KX::ServerState serverState(serverK);
    EXPECT(serverState.setM1(m1.data()));
    while (serverState.update()) {}
    std::array<u8, KX::M2Size> m2;
    EXPECT(serverState.getM2(m2.data()));
    const Array<u8, 32> *clientPK = serverState.clientPK();
    EXPECT(clientPK);
    const Session *serverSession = serverState.serverSession();
    EXPECT(serverSession);
    EXPECT(clientState.setM2(m2.data()));
    while (clientState.update()) {}
    const Session *clientSession = clientState.clientSession();
    EXPECT(clientSession);

    Array<u8, 32> expectedClientPK;
    crypto_x25519_public_key(expectedClientPK.values(), clientK.values());
    EXPECT(*clientPK == expectedClientPK);
    EXPECT(serverSession->m_readK != serverSession->m_writeK);
    EXPECT(serverSession->m_readK == clientSession->m_writeK);
    EXPECT(serverSession->m_writeK == clientSession->m_readK);
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
