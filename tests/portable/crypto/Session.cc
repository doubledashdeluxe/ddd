#include <native/crypto/Random.hh>
#include <portable/crypto/Session.hh>
#include <snitch/snitch_all.hpp>

TEST_CASE("Session") {
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
        CHECK_FALSE(serverSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        clientSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(mac, sizeof(mac));
        CHECK_FALSE(serverSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        clientSession.write(buffer, sizeof(buffer), mac, nonce);
        CHECK(serverSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        serverSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(buffer, sizeof(buffer));
        CHECK_FALSE(clientSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        serverSession.write(buffer, sizeof(buffer), mac, nonce);
        Random::Get(mac, sizeof(mac));
        CHECK_FALSE(clientSession.read(buffer, sizeof(buffer), mac, nonce));

        Random::Get(buffer, sizeof(buffer));
        serverSession.write(buffer, sizeof(buffer), mac, nonce);
        CHECK(clientSession.read(buffer, sizeof(buffer), mac, nonce));
    }
}
