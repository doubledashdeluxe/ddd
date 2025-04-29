#include "FakeSZSReader.hh"

FakeSZSReader::FakeSZSReader(const std::vector<u8> *compressed) : m_compressed(compressed) {}

FakeSZSReader::~FakeSZSReader() = default;

bool FakeSZSReader::read() {
    return SZSReader::read();
}

u32 FakeSZSReader::uncompressedSize() const {
    return m_uncompressedSize;
}

const std::vector<u8> &FakeSZSReader::uncompressed() const {
    return m_uncompressed;
}

bool FakeSZSReader::setSize(u32 size) {
    m_uncompressedSize = size;
    return true;
}

bool FakeSZSReader::read(const u8 *&buffer, u32 &size) {
    if (!m_compressed) {
        return false;
    }
    buffer = m_compressed->data();
    size = m_compressed->size();
    m_compressed = nullptr;
    return true;
}

bool FakeSZSReader::write(const u8 *buffer, u32 size) {
    m_uncompressed.insert(m_uncompressed.end(), buffer, buffer + size);
    return true;
}
