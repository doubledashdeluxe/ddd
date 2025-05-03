#include "FakeUDPSocket.hh"

#include <cstring>

FakeUDPSocket::FakeUDPSocket(std::vector<u8> &data) : m_data(data) {}

FakeUDPSocket::~FakeUDPSocket() = default;

s32 FakeUDPSocket::open() {
    return 0;
}

s32 FakeUDPSocket::close() {
    return 0;
}

bool FakeUDPSocket::ok() {
    return true;
}

s32 FakeUDPSocket::recvFrom(void *buffer, u32 size, Address &address) {
    if (m_data.size() > size || !m_address) {
        return -1;
    }

    if (m_data.size() != 0) {
        memcpy(buffer, m_data.data(), m_data.size());
    }
    address = *m_address;
    return m_data.size();
}

s32 FakeUDPSocket::sendTo(const void * /* buffer */, u32 size, const Address &address) {
    m_address = address;
    return size;
}
