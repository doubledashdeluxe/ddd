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

    std::array<u8, KX::M1Size> m1;
    KX::ClientState clientState;
    KX::IK1(clientK, invalidServerPK, m1.data(), clientState);
    std::array<u8, KX::M2Size> m2;
    Array<u8, 32> clientPK;
    Session serverSession;
    EXPECT_NOT(KX::IK2(serverK, m1.data(), m2.data(), clientPK, serverSession));
}

CASE("Invalid m1") {
    Array<u8, 32> clientK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    std::array<u8, KX::M1Size> m1;
    KX::ClientState clientState;
    KX::IK1(clientK, serverPK, m1.data(), clientState);
    std::array<u8, KX::M1Size> invalidM1;
    Random::Get(invalidM1.data(), invalidM1.size());
    std::array<u8, KX::M2Size> m2;
    Array<u8, 32> clientPK;
    Session serverSession;
    EXPECT_NOT(KX::IK2(serverK, invalidM1.data(), m2.data(), clientPK, serverSession));
}

CASE("Invalid m2") {
    Array<u8, 32> clientK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    std::array<u8, KX::M1Size> m1;
    KX::ClientState clientState;
    KX::IK1(clientK, serverPK, m1.data(), clientState);
    std::array<u8, KX::M2Size> m2;
    Array<u8, 32> clientPK;
    Session serverSession;
    EXPECT(KX::IK2(serverK, m1.data(), m2.data(), clientPK, serverSession));
    std::array<u8, KX::M2Size> invalidM2;
    Random::Get(invalidM2.data(), invalidM2.size());
    Session clientSession;
    EXPECT_NOT(KX::IK3(clientState, invalidM2.data(), clientSession));
}

CASE("Valid") {
    Array<u8, 32> clientK, serverK, serverPK;
    Random::Get(clientK.values(), clientK.count());
    Random::Get(serverK.values(), serverK.count());
    crypto_x25519_public_key(serverPK.values(), serverK.values());

    std::array<u8, KX::M1Size> m1;
    KX::ClientState clientState;
    KX::IK1(clientK, serverPK, m1.data(), clientState);
    std::array<u8, KX::M2Size> m2;
    Array<u8, 32> clientPK;
    Session serverSession;
    EXPECT(KX::IK2(serverK, m1.data(), m2.data(), clientPK, serverSession));
    Session clientSession;
    EXPECT(KX::IK3(clientState, m2.data(), clientSession));

    Array<u8, 32> expectedClientPK;
    crypto_x25519_public_key(expectedClientPK.values(), clientK.values());
    EXPECT(clientPK == expectedClientPK);
    EXPECT(serverSession.m_readK != serverSession.m_writeK);
    EXPECT(serverSession.m_readK == clientSession.m_writeK);
    EXPECT(serverSession.m_writeK == clientSession.m_readK);
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
