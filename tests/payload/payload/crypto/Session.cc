#include <lest.hpp>
#include <payload/crypto/Random.hh>
#include <payload/crypto/Session.hh>

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("Session") {
    Session clientSession;
    Random::Get(clientSession.m_readK.values(), clientSession.m_readK.count());
    Random::Get(clientSession.m_writeK.values(), clientSession.m_writeK.count());

    Session serverSession;
    serverSession.m_readK = clientSession.m_writeK;
    serverSession.m_writeK = clientSession.m_readK;

    for (size_t i = 0; i < 16; i++) {
        u8 mac[Session::MACSize], nonce[Session::NonceSize], buffer[128];

        Random::Get(buffer, sizeof(buffer));
        clientSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(buffer, sizeof(buffer));
        EXPECT_NOT(serverSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        clientSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(mac, sizeof(mac));
        EXPECT_NOT(serverSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        clientSession.write(buffer, sizeof(buffer), mac, nonce);
        EXPECT(serverSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        serverSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(buffer, sizeof(buffer));
        EXPECT_NOT(clientSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        serverSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(mac, sizeof(mac));
        EXPECT_NOT(clientSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        serverSession.write(buffer, sizeof(buffer), mac, nonce);
        EXPECT(clientSession.read(buffer, sizeof(buffer), mac, nonce));
    }
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
