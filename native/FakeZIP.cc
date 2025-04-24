#include "FakeZIP.hh"

FakeZIP::FakeZIP(std::vector<u8> &data) : m_data(data) {
    setup();
}

FakeZIP::~FakeZIP() = default;

bool FakeZIP::read(void *dst, u32 size, u32 offset) {
    memcpy(dst, m_data.data() + offset, size);
    return true;
}

bool FakeZIP::write(const void *src, u32 size, u32 offset) {
    if (offset + size > m_data.size()) {
        m_data.resize(offset + size);
    }
    memcpy(m_data.data() + offset, src, size);
    return true;
}

bool FakeZIP::truncate(u64 size) {
    m_data.resize(size);
    return true;
}

bool FakeZIP::size(u64 &size) {
    size = m_data.size();
    return true;
}

u32 FakeZIP::getDOSTime() {
    return 0;
}
