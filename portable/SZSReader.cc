#include "SZSReader.hh"

SZSReader::SZSReader() : m_compressedBufferSize(0), m_uncompressedBufferOffset(0) {}

SZSReader::~SZSReader() {}

bool SZSReader::read() {
    for (u32 i = 0; i < 4; i++) {
        u8 value;
        if (!read(value) || (value != "Yaz0"[i] && value != "Yaz1"[i])) {
            return false;
        }
    }

    u32 uncompressedSize = 0;
    for (u32 i = 0; i < 4; i++) {
        u8 value;
        if (!read(value)) {
            return false;
        }
        uncompressedSize <<= 8;
        uncompressedSize |= value;
    }
    if (!setSize(uncompressedSize)) {
        return false;
    }

    for (u32 i = 0; i < 8; i++) {
        u8 value;
        if (!read(value)) {
            return false;
        }
    }

    u32 uncompressedOffset = 0;
    u8 groupHeader;
    for (u32 i = 0; uncompressedOffset < uncompressedSize; i = (i + 1) % 8) {
        u8 value;
        if (!read(value)) {
            break;
        }
        if (i == 0) {
            groupHeader = value;
            if (!read(value)) {
                return false;
            }
        }
        if (groupHeader >> (7 - i) & 1) {
            if (!write(value)) {
                return false;
            }
            uncompressedOffset++;
        } else {
            u16 referenceHeader = value << 8;
            if (!read(value)) {
                return false;
            }
            referenceHeader |= value << 0;
            u16 referenceOffset = (referenceHeader & 0xfff) + 0x1;
            if (referenceOffset > uncompressedOffset) {
                return false;
            }
            u16 referenceSize = (referenceHeader >> 12) + 0x2;
            if (referenceSize == 0x2) {
                if (!read(value)) {
                    return false;
                }
                referenceSize = value + 0x12;
            }
            if (uncompressedOffset + referenceSize < uncompressedOffset) {
                return false;
            }
            if (uncompressedOffset + referenceSize > uncompressedSize) {
                return false;
            }
            referenceOffset = m_uncompressedBufferOffset - referenceOffset;
            for (u32 j = 0; j < referenceSize; j++) {
                referenceOffset %= m_uncompressedBuffer.count();
                u8 value = m_uncompressedBuffer[referenceOffset];
                if (!write(value)) {
                    return false;
                }
                referenceOffset++;
            }
            uncompressedOffset += referenceSize;
        }
    }

    if (!write(m_uncompressedBuffer.values(), m_uncompressedBufferOffset)) {
        return false;
    }

    return uncompressedOffset == uncompressedSize;
}

bool SZSReader::read(u8 &value) {
    while (m_compressedBufferSize == 0) {
        if (!read(m_compressedBuffer, m_compressedBufferSize)) {
            return false;
        }
    }
    value = *m_compressedBuffer++;
    m_compressedBufferSize--;
    return true;
}

bool SZSReader::write(u8 value) {
    m_uncompressedBuffer[m_uncompressedBufferOffset++] = value;
    if (m_uncompressedBufferOffset < m_uncompressedBuffer.count()) {
        return true;
    }
    m_uncompressedBufferOffset = 0;
    return write(m_uncompressedBuffer.values(), m_uncompressedBuffer.count());
}
