#include "FakeDNS.hh"

#include <cstring>

FakeDNS::FakeDNS(std::vector<u8> &data) : m_data(data) {}

FakeDNS::~FakeDNS() = default;

bool FakeDNS::ok() {
    return true;
}

s32 FakeDNS::open() {
    return 0;
}

s32 FakeDNS::recvFrom(void *buffer, u32 size, u32 &address) {
    if (m_data.size() > size || !m_address) {
        return -1;
    }

    memcpy(buffer, m_data.data(), m_data.size());
    address = *m_address;
    return m_data.size();
}

s32 FakeDNS::sendTo(const void * /* buffer */, u32 size, u32 address) {
    m_address = address;
    return size;
}

s64 FakeDNS::secondsToTicks(s64 seconds) {
    return seconds;
}

s64 FakeDNS::getMonotonicTicks() {
    return m_ticks++;
}
