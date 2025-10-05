#include "ServerConnectionStateIdle.hh"

#include "native/online/ServerConnectionStateKX.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <string.h>
}

ServerConnectionStateIdle::ServerConnectionStateIdle(const ServerPlatform &platform)
    : ServerConnectionState(platform) {
    m_platform.random.get(m_cookie.data(), m_cookie.size());
}

ServerConnectionStateIdle::~ServerConnectionStateIdle() = default;

ServerConnectionState &ServerConnectionStateIdle::transfer(const u8 *input, size_t inputSize,
        std::vector<u8> &output) {
    if (inputSize != m_cookie.size() + KX::M1Size) {
        return *this;
    }

    if (memcmp(input, m_cookie.data(), m_cookie.size())) {
        output.insert(output.begin(), m_cookie.begin(), m_cookie.end());
        return *this;
    }
    input += m_cookie.size();

    Array<u8, 32> serverEphemeralK;
    m_platform.random.get(serverEphemeralK.values(), serverEphemeralK.count());
    KX::ServerState serverState(m_platform.serverK, serverEphemeralK);
    crypto_wipe(serverEphemeralK.values(), serverEphemeralK.count());
    if (!serverState.setM1(input)) {
        return *this;
    }

    while (serverState.update()) {}
    std::array<u8, KX::M2Size> m2;
    if (!serverState.getM2(m2.data())) {
        return *this;
    }

    auto *session = serverState.serverSession();
    if (!session) {
        return *this;
    }

    return *(new (m_platform.allocator) ServerConnectionStateKX(m_platform, m2, *session));
}
