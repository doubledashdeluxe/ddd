#include "OnlineFakeUDPSocket.hh"

OnlineFakeUDPSocket::OnlineFakeUDPSocket(const ServerPlatform &platform)
    : FakeUDPSocket(m_data), m_connection(platform) {}

OnlineFakeUDPSocket::~OnlineFakeUDPSocket() = default;

s32 OnlineFakeUDPSocket::sendTo(const void *buffer, u32 size, const Address &address) {
    m_connection.transfer(static_cast<const u8 *>(buffer), size, m_data);

    return FakeUDPSocket::sendTo(buffer, size, address);
}
