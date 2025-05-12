#include "ServerConnectionStateKX.hh"

#include "native/online/ServerConnectionStateSession.hh"

#include <vector>

ServerConnectionStateKX::ServerConnectionStateKX(const ServerPlatform &platform,
        const std::array<u8, KX::M2Size> &m2, const Session &session)
    : ServerConnectionState(platform), m_m2(m2), m_session(session) {}

ServerConnectionStateKX::~ServerConnectionStateKX() = default;

ServerConnectionState &ServerConnectionStateKX::transfer(const u8 *input, size_t inputSize,
        std::vector<u8> &output) {
    if (inputSize >= Session::MACSize + Session::NonceSize) {
        inputSize -= Session::MACSize + Session::NonceSize;
        const u8 *mac = input + inputSize;
        const u8 *nonce = mac + Session::MACSize;
        std::vector<u8> copy(input, input + inputSize);
        if (m_session.read(copy.data(), copy.size(), mac, nonce)) {
            return *(
                    new (m_platform.allocator) ServerConnectionStateSession(m_platform, m_session));
        }
    }

    output.insert(output.begin(), m_m2.begin(), m_m2.end());
    return *this;
}
