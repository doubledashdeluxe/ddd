#include "ServerConnectionStateSession.hh"

ServerConnectionStateSession::ServerConnectionStateSession(const ServerPlatform &platform,
        Session session)
    : ServerConnectionState(platform), m_session(session) {}

ServerConnectionStateSession::~ServerConnectionStateSession() = default;

ServerConnectionState &ServerConnectionStateSession::transfer(const u8 * /* input */,
        size_t /* inputSize */, std::vector<u8> &output) {
    output = m_platform.data;
    u32 size = output.size();
    output.resize(size + Session::MACSize + Session::NonceSize);
    u8 *mac = output.data() + size;
    u8 *nonce = mac + Session::MACSize;
    m_session.write(output.data(), size, mac, nonce);
    return *this;
}
