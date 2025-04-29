#include "ZIP.hh"

#include "portable/Algorithm.hh"
#include "portable/Bytes.hh"

extern "C" {
#include <string.h>
}

ZIP::Reader::Reader(ZIP &zip, const char *path) : m_ok(false), m_zip(zip) {
    if (!zip.m_ok) {
        return;
    }

    u32 cdNodeIndex, cdNodeOffset;
    for (cdNodeIndex = 0, cdNodeOffset = zip.m_eocd.cdOffset; cdNodeIndex < zip.m_eocd.cdNodeCount;
            cdNodeIndex++, cdNodeOffset = m_cdNode.nextOffset) {
        if (!zip.readCDNode(cdNodeOffset, m_cdNode)) {
            return;
        }

        if (path[0] == '/') {
            if (!strcmp(m_cdNode.path.values(), path + 1)) {
                break;
            }
        } else {
            char *cdNodeName = strrchr(m_cdNode.path.values(), '/');
            cdNodeName = cdNodeName ? cdNodeName + 1 : m_cdNode.path.values();
            if (!strcmp(cdNodeName, path)) {
                break;
            }
        }
    }
    if (cdNodeIndex == zip.m_eocd.cdNodeCount) {
        return;
    }

    if (!zip.readLocalNode(m_cdNode.localNodeOffset, m_localNode)) {
        return;
    }

    switch (m_cdNode.compressionMethod) {
    case CompressionMethod::Store:
        break;
    case CompressionMethod::Deflate:
        tinfl_init(&m_decompressor);
        m_status = TINFL_STATUS_NEEDS_MORE_INPUT;
        m_compressedOffset = 0;
        break;
    default:
        return;
    }

    m_uncompressedOffset = 0;
    m_crc32 = MZ_CRC32_INIT;

    m_ok = true;
}

ZIP::Reader::~Reader() {}

bool ZIP::Reader::ok() const {
    return m_ok;
}

const ZIP::CDNode *ZIP::Reader::cdNode() const {
    return m_ok ? &m_cdNode : static_cast<CDNode *>(nullptr);
}

const ZIP::LocalNode *ZIP::Reader::localNode() const {
    return m_ok ? &m_localNode : static_cast<LocalNode *>(nullptr);
}

const u32 *ZIP::Reader::size() const {
    return m_ok ? &m_cdNode.uncompressedSize : static_cast<u32 *>(nullptr);
}

bool ZIP::Reader::read(const u8 *&buffer, u32 &size) {
    if (!m_ok) {
        return false;
    }

    u8 *uncompressed;
    size_t uncompressedSize;
    if (m_cdNode.compressionMethod == CompressionMethod::Store) {
        uncompressed = m_uncompressedBuffer.values();
        uncompressedSize = m_cdNode.uncompressedSize - m_uncompressedOffset;
        if (uncompressedSize > m_uncompressedBuffer.count()) {
            uncompressedSize = m_uncompressedBuffer.count();
        }
        if (!m_zip.read(uncompressed, uncompressedSize,
                    m_localNode.compressedOffset + m_uncompressedOffset)) {
            return m_ok = false;
        }
        m_uncompressedOffset += uncompressedSize;
    } else {
        s32 flags = 0;
        u32 compressedBufferOffset = m_compressedOffset % m_compressedBuffer.count();
        u8 *compressed = m_compressedBuffer.values() + compressedBufferOffset;
        size_t compressedSize = m_compressedBuffer.count() - compressedBufferOffset;
        if (compressedSize > m_cdNode.compressedSize - m_compressedOffset) {
            compressedSize = m_cdNode.compressedSize - m_compressedOffset;
        } else {
            flags |= TINFL_FLAG_HAS_MORE_INPUT;
        }
        u32 uncompressedBufferOffset = m_uncompressedOffset % m_uncompressedBuffer.count();
        uncompressed = m_uncompressedBuffer.values() + uncompressedBufferOffset;
        uncompressedSize = m_uncompressedBuffer.count() - uncompressedBufferOffset;
        if (m_status == TINFL_STATUS_NEEDS_MORE_INPUT) {
            if (!m_zip.read(compressed, compressedSize,
                        m_localNode.compressedOffset + m_compressedOffset)) {
                return m_ok = false;
            }
        }
        m_status = tinfl_decompress(&m_decompressor, compressed, &compressedSize,
                m_uncompressedBuffer.values(), uncompressed, &uncompressedSize, flags);
        m_compressedOffset += compressedSize;
        m_uncompressedOffset += uncompressedSize;
        if (m_compressedOffset > m_cdNode.compressedSize) {
            return m_ok = false;
        }
        if (m_uncompressedOffset > m_cdNode.uncompressedSize) {
            return m_ok = false;
        }
        switch (m_status) {
        case TINFL_STATUS_DONE:
            if (m_compressedOffset != m_cdNode.compressedSize) {
                return m_ok = false;
            }
            if (m_uncompressedOffset != m_cdNode.uncompressedSize) {
                return m_ok = false;
            }
            break;
        case TINFL_STATUS_NEEDS_MORE_INPUT:
            break;
        case TINFL_STATUS_HAS_MORE_OUTPUT:
            if (m_compressedOffset == m_cdNode.compressedSize) {
                return m_ok = false;
            }
            break;
        default:
            return m_ok = false;
        }
    }
    m_crc32 = mz_crc32(m_crc32, uncompressed, uncompressedSize);
    if (m_uncompressedOffset == m_cdNode.uncompressedSize) {
        if (m_crc32 != m_cdNode.uncompressedCRC32) {
            return false;
        }
    }
    buffer = uncompressed;
    size = uncompressedSize;
    return true;
}

ZIP::Writer::Writer(ZIP &zip, const char *path, u32 size)
    : m_ok(false), m_zip(zip), m_size(size) {
    if (path[0] != '/') {
        return;
    }

    m_eocd.cdNodeCount = zip.m_eocd.cdNodeCount + 1;
    if (m_eocd.cdNodeCount < zip.m_eocd.cdNodeCount) {
        return;
    }

    u32 cdNodeIndex;
    for (cdNodeIndex = 0, m_cdNodeOffset = zip.m_eocd.cdOffset;
            cdNodeIndex < zip.m_eocd.cdNodeCount;
            cdNodeIndex++, m_cdNodeOffset = m_cdNode.nextOffset) {
        if (!zip.readCDNode(m_cdNodeOffset, m_cdNode)) {
            return;
        }

        if (!strcmp(m_cdNode.path.values(), path + 1)) {
            break;
        }
    }
    m_isNew = cdNodeIndex == zip.m_eocd.cdNodeCount;

    m_offset = zip.m_fileSize;
    m_localNode.compressionMethod = CompressionMethod::Store;
    u32 dosTime = zip.getDOSTime();
    m_localNode.time = dosTime >> 0;
    m_localNode.date = dosTime >> 16;
    m_localNode.uncompressedCRC32 = MZ_CRC32_INIT;
    m_localNode.compressedSize = m_size;
    m_localNode.uncompressedSize = m_size;
    s32 pathLength = snprintf(m_localNode.path.values(), m_localNode.path.count(), "%s", path + 1);
    if (pathLength >= static_cast<s32>(m_localNode.path.count())) {
        return;
    }
    m_localNode.compressedOffset = m_offset + LocalNodeHeaderSize + pathLength;
    if (m_offset + LocalNodeHeaderSize < m_offset) {
        return;
    }
    if (m_localNode.compressedOffset < m_offset) {
        return;
    }
    if (!zip.writeLocalNode(m_offset, m_localNode)) {
        return;
    }
    m_offset = m_localNode.compressedOffset;

    m_ok = true;
}

ZIP::Writer::~Writer() {
    m_zip.truncate(m_zip.m_fileSize);
}

bool ZIP::Writer::ok() const {
    return m_ok;
}

const ZIP::CDNode *ZIP::Writer::cdNode() const {
    return m_ok ? &m_cdNode : static_cast<CDNode *>(nullptr);
}

const ZIP::LocalNode *ZIP::Writer::localNode() const {
    return m_ok ? &m_localNode : static_cast<LocalNode *>(nullptr);
}

bool ZIP::Writer::write(const u8 *buffer, u32 size) {
    if (!m_ok) {
        return false;
    }
    if (size > m_size) {
        return m_ok = false;
    }
    if (m_offset + size < m_offset) {
        return m_ok = false;
    }
    m_localNode.uncompressedCRC32 = mz_crc32(m_localNode.uncompressedCRC32, buffer, size);
    if (!m_zip.write(buffer, size, m_offset)) {
        return m_ok = false;
    }
    m_size -= size;
    m_offset += size;
    if (m_size != 0) {
        return true;
    }

    m_eocd.cdOffset = m_offset;

    if (!m_zip.writeLocalNode(m_zip.m_fileSize, m_localNode)) {
        return m_ok = false;
    }

    if (m_isNew) {
        if (!m_zip.copy(m_zip.m_eocd.cdOffset, m_offset, m_zip.m_eocd.cdSize)) {
            return m_ok = false;
        }
        m_offset += m_zip.m_eocd.cdSize;
    } else {
        if (!m_zip.copy(m_zip.m_eocd.cdOffset, m_offset, m_cdNodeOffset - m_zip.m_eocd.cdOffset)) {
            return m_ok = false;
        }
        m_offset += m_cdNodeOffset - m_zip.m_eocd.cdOffset;
        if (!m_zip.copy(m_cdNode.nextOffset, m_offset,
                    m_zip.m_eocd.cdSize - (m_cdNodeOffset - m_zip.m_eocd.cdOffset))) {
            return m_ok = false;
        }
        m_offset += m_eocd.cdSize - (m_cdNodeOffset - m_eocd.cdOffset);
    }

    m_cdNode.compressionMethod = m_localNode.compressionMethod;
    m_cdNode.time = m_localNode.time;
    m_cdNode.date = m_localNode.date;
    m_cdNode.uncompressedCRC32 = m_localNode.uncompressedCRC32;
    m_cdNode.compressedSize = m_localNode.compressedSize;
    m_cdNode.uncompressedSize = m_localNode.uncompressedSize;
    m_cdNode.localNodeOffset = m_zip.m_fileSize;
    m_cdNode.path = m_localNode.path;
    u32 pathLength = strlen(m_localNode.path.values());
    m_cdNode.nextOffset = m_offset + CDNodeHeaderSize + pathLength;
    if (m_offset + CDNodeHeaderSize < m_offset) {
        return m_ok = false;
    }
    if (m_cdNode.nextOffset < m_offset) {
        return m_ok = false;
    }
    if (!m_zip.writeCDNode(m_offset, m_cdNode)) {
        return m_ok = false;
    }
    m_offset = m_cdNode.nextOffset;
    m_eocd.cdSize = m_offset - m_eocd.cdOffset;

    if (m_offset + EOCDHeaderSize < m_offset) {
        return m_ok = false;
    }
    if (!m_zip.writeEOCD(m_offset, m_eocd)) {
        return m_ok = false;
    }
    m_offset += EOCDHeaderSize;

    m_zip.m_fileSize = m_offset;
    m_zip.m_eocd = m_eocd;
    return true;
}

ZIP::ZIP() : m_ok(false) {}

ZIP::~ZIP() {}

bool ZIP::ok() const {
    return m_ok;
}

const ZIP::EOCD *ZIP::eocd() const {
    return m_ok ? &m_eocd : static_cast<EOCD *>(nullptr);
}

void ZIP::setup() {
    u64 fileSize;
    if (!size(fileSize)) {
        return;
    }
    if (fileSize > UINT32_MAX) {
        return;
    }
    m_fileSize = fileSize;

    if (!readEOCD(m_eocd)) {
        return;
    }

    m_ok = true;
}

bool ZIP::readEOCD(EOCD &eocd) {
    if (m_fileSize < EOCDHeaderSize) {
        return false;
    }

    Array<u8, EOCDHeaderSize> eocdHeader;
    u32 minEOCDOffset = m_fileSize > MaxEOCDCommentSize ? m_fileSize - MaxEOCDCommentSize : 0;
    u32 maxEOCDOffset = m_fileSize - EOCDHeaderSize;
    u32 eocdOffset = maxEOCDOffset;
    if (!read(eocdHeader.values(), eocdHeader.count(), eocdOffset)) {
        return false;
    }
    while (!readEOCD(eocdOffset, eocdHeader, eocd)) {
        if (eocdOffset == minEOCDOffset) {
            return false;
        }

        eocdOffset--;
        eocdHeader.rotateRight(1);
        if (!read(eocdHeader.values(), 1, eocdOffset)) {
            return false;
        }
    }

    return true;
}

bool ZIP::readEOCD(u32 eocdOffset, const Array<u8, EOCDHeaderSize> &eocdHeader, EOCD &eocd) {
    u32 eocdSignature;
    memcpy(&eocdSignature, eocdHeader.values(), sizeof(eocdSignature));
    if (memcmp(&eocdSignature, "PK\5\6", sizeof(eocdSignature))) {
        return false;
    }

    u16 currDiskIndex = Bytes::ReadLE<u16>(eocdHeader.values(), 0x04);
    if (currDiskIndex != 0) {
        return false;
    }

    u16 cdFirstDisk = Bytes::ReadLE<u16>(eocdHeader.values(), 0x06);
    if (cdFirstDisk != 0) {
        return false;
    }

    u16 cdCurrDiskNodeCount = Bytes::ReadLE<u16>(eocdHeader.values(), 0x08);
    eocd.cdNodeCount = Bytes::ReadLE<u16>(eocdHeader.values(), 0x0a);
    if (cdCurrDiskNodeCount != eocd.cdNodeCount) {
        return false;
    }

    eocd.cdSize = Bytes::ReadLE<u32>(eocdHeader.values(), 0x0c);
    eocd.cdOffset = Bytes::ReadLE<u32>(eocdHeader.values(), 0x10);
    if (eocd.cdOffset > eocdOffset) {
        return false;
    }
    if (eocd.cdOffset + eocd.cdSize != eocdOffset) {
        return false;
    }

    u16 eocdCommentSize = Bytes::ReadLE<u16>(eocdHeader.values(), 0x14);
    if (eocdOffset + EOCDHeaderSize + eocdCommentSize != m_fileSize) {
        return false;
    }

    return true;
}

bool ZIP::readCDNode(u32 cdNodeOffset, CDNode &cdNode) {
    if (cdNodeOffset > m_eocd.cdOffset + m_eocd.cdSize) {
        return false;
    }
    if (cdNodeOffset + CDNodeHeaderSize > m_eocd.cdOffset + m_eocd.cdSize) {
        return false;
    }

    Array<u8, CDNodeHeaderSize> cdNodeHeader;
    if (!read(cdNodeHeader.values(), cdNodeHeader.count(), cdNodeOffset)) {
        return false;
    }

    u32 cdNodeSignature;
    memcpy(&cdNodeSignature, cdNodeHeader.values(), sizeof(cdNodeSignature));
    if (memcmp(&cdNodeSignature, "PK\1\2", sizeof(cdNodeSignature))) {
        return false;
    }

    u32 cdNodePathOffset = cdNodeOffset + CDNodeHeaderSize;
    if (cdNodePathOffset < cdNodeOffset) {
        return false;
    }
    if (cdNodePathOffset > m_eocd.cdOffset + m_eocd.cdSize) {
        return false;
    }
    u16 cdNodePathSize = Bytes::ReadLE<u16>(cdNodeHeader.values(), 0x1c);

    u32 cdNodeExtraFieldOffset = cdNodePathOffset + cdNodePathSize;
    if (cdNodeExtraFieldOffset < cdNodePathOffset) {
        return false;
    }
    if (cdNodeExtraFieldOffset > m_eocd.cdOffset + m_eocd.cdSize) {
        return false;
    }
    u16 cdNodeExtraFieldSize = Bytes::ReadLE<u16>(cdNodeHeader.values(), 0x1e);

    u32 cdNodeCommentOffset = cdNodeExtraFieldOffset + cdNodeExtraFieldSize;
    if (cdNodeCommentOffset < cdNodeExtraFieldOffset) {
        return false;
    }
    if (cdNodeCommentOffset > m_eocd.cdOffset + m_eocd.cdSize) {
        return false;
    }
    u16 cdNodeCommentSize = Bytes::ReadLE<u16>(cdNodeHeader.values(), 0x20);

    cdNode.nextOffset = cdNodeCommentOffset + cdNodeCommentSize;
    if (cdNode.nextOffset < cdNodeCommentOffset) {
        return false;
    }
    if (cdNode.nextOffset > m_eocd.cdOffset + m_eocd.cdSize) {
        return false;
    }

    if (cdNodePathSize >= cdNode.path.count()) {
        return false;
    }
    if (!read(cdNode.path.values(), cdNodePathSize, cdNodePathOffset)) {
        return false;
    }
    cdNode.path[cdNodePathSize] = '\0';
    if (strnlen(cdNode.path.values(), cdNode.path.count()) != cdNodePathSize) {
        return false;
    }

    cdNode.compressionMethod = Bytes::ReadLE<u16>(cdNodeHeader.values(), 0x0a);
    cdNode.time = Bytes::ReadLE<u16>(cdNodeHeader.values(), 0x0c);
    cdNode.date = Bytes::ReadLE<u16>(cdNodeHeader.values(), 0x0e);
    cdNode.uncompressedCRC32 = Bytes::ReadLE<u32>(cdNodeHeader.values(), 0x10);
    cdNode.compressedSize = Bytes::ReadLE<u32>(cdNodeHeader.values(), 0x14);
    cdNode.uncompressedSize = Bytes::ReadLE<u32>(cdNodeHeader.values(), 0x18);
    cdNode.localNodeOffset = Bytes::ReadLE<u32>(cdNodeHeader.values(), 0x2a);

    return true;
}

bool ZIP::readLocalNode(u32 localNodeOffset, LocalNode &localNode) {
    if (localNodeOffset > m_eocd.cdOffset) {
        return false;
    }
    if (localNodeOffset + LocalNodeHeaderSize > m_eocd.cdOffset) {
        return false;
    }

    Array<u8, LocalNodeHeaderSize> localNodeHeader;
    if (!read(localNodeHeader.values(), localNodeHeader.count(), localNodeOffset)) {
        return false;
    }

    u32 localNodeSignature;
    memcpy(&localNodeSignature, localNodeHeader.values(), sizeof(localNodeSignature));
    if (memcmp(&localNodeSignature, "PK\3\4", sizeof(localNodeSignature))) {
        return false;
    }

    u32 localNodePathOffset = localNodeOffset + LocalNodeHeaderSize;
    if (localNodePathOffset < localNodeOffset) {
        return false;
    }
    if (localNodePathOffset > m_eocd.cdOffset) {
        return false;
    }
    u16 localNodePathSize = Bytes::ReadLE<u16>(localNodeHeader.values(), 0x1a);

    u32 localNodeExtraFieldOffset = localNodePathOffset + localNodePathSize;
    if (localNodeExtraFieldOffset < localNodePathOffset) {
        return false;
    }
    if (localNodeExtraFieldOffset > m_eocd.cdOffset) {
        return false;
    }
    u16 localNodeExtraFieldSize = Bytes::ReadLE<u16>(localNodeHeader.values(), 0x1c);

    localNode.compressedOffset = localNodeExtraFieldOffset + localNodeExtraFieldSize;
    if (localNode.compressedOffset < localNodeExtraFieldOffset) {
        return false;
    }
    if (localNode.compressedOffset > m_eocd.cdOffset) {
        return false;
    }

    if (localNodePathSize >= localNode.path.count()) {
        return false;
    }
    if (!read(localNode.path.values(), localNodePathSize, localNodePathOffset)) {
        return false;
    }
    localNode.path[localNodePathSize] = '\0';
    if (strnlen(localNode.path.values(), localNode.path.count()) != localNodePathSize) {
        return false;
    }

    localNode.compressionMethod = Bytes::ReadLE<u16>(localNodeHeader.values(), 0x08);
    localNode.time = Bytes::ReadLE<u16>(localNodeHeader.values(), 0x0a);
    localNode.date = Bytes::ReadLE<u16>(localNodeHeader.values(), 0x0c);
    localNode.uncompressedCRC32 = Bytes::ReadLE<u32>(localNodeHeader.values(), 0x0e);
    localNode.compressedSize = Bytes::ReadLE<u32>(localNodeHeader.values(), 0x12);
    localNode.uncompressedSize = Bytes::ReadLE<u32>(localNodeHeader.values(), 0x16);

    return true;
}

bool ZIP::writeEOCD(u32 eocdOffset, const EOCD &eocd) {
    Array<u8, EOCDHeaderSize> eocdHeader;
    memset(eocdHeader.values(), 0, eocdHeader.count());
    u32 eocdSignature;
    memcpy(&eocdSignature, "PK\5\6", sizeof(eocdSignature));
    memcpy(&eocdHeader[0x00], &eocdSignature, sizeof(eocdSignature));
    Bytes::WriteLE<u16>(eocdHeader.values(), 0x08, eocd.cdNodeCount);
    Bytes::WriteLE<u16>(eocdHeader.values(), 0x0a, eocd.cdNodeCount);
    Bytes::WriteLE<u32>(eocdHeader.values(), 0x0c, eocd.cdSize);
    Bytes::WriteLE<u32>(eocdHeader.values(), 0x10, eocd.cdOffset);

    if (!write(eocdHeader.values(), eocdHeader.count(), eocdOffset)) {
        return false;
    }

    return true;
}

bool ZIP::writeCDNode(u32 cdNodeOffset, const CDNode &cdNode) {
    Array<u8, CDNodeHeaderSize> cdNodeHeader;
    memset(cdNodeHeader.values(), 0, cdNodeHeader.count());
    u32 cdNodeSignature;
    memcpy(&cdNodeSignature, "PK\1\2", sizeof(cdNodeSignature));
    memcpy(&cdNodeHeader[0x00], &cdNodeSignature, sizeof(cdNodeSignature));
    Bytes::WriteLE<u16>(cdNodeHeader.values(), 0x0a, cdNode.compressionMethod);
    Bytes::WriteLE<u16>(cdNodeHeader.values(), 0x0c, cdNode.time);
    Bytes::WriteLE<u16>(cdNodeHeader.values(), 0x0e, cdNode.date);
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x10, cdNode.uncompressedCRC32);
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x14, cdNode.compressedSize);
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x18, cdNode.uncompressedSize);
    u32 cdNodePathSize = strlen(cdNode.path.values());
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x1c, cdNodePathSize);
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x2a, cdNode.localNodeOffset);

    if (!write(cdNodeHeader.values(), cdNodeHeader.count(), cdNodeOffset)) {
        return false;
    }

    if (!write(cdNode.path.values(), cdNodePathSize, cdNodeOffset + cdNodeHeader.count())) {
        return false;
    }

    return true;
}

bool ZIP::writeLocalNode(u32 localNodeOffset, const LocalNode &localNode) {
    Array<u8, LocalNodeHeaderSize> localNodeHeader;
    memset(localNodeHeader.values(), 0, localNodeHeader.count());
    u32 localNodeSignature;
    memcpy(&localNodeSignature, "PK\3\4", sizeof(localNodeSignature));
    memcpy(&localNodeHeader[0x00], &localNodeSignature, sizeof(localNodeSignature));
    Bytes::WriteLE<u16>(localNodeHeader.values(), 0x08, localNode.compressionMethod);
    Bytes::WriteLE<u16>(localNodeHeader.values(), 0x0a, localNode.time);
    Bytes::WriteLE<u16>(localNodeHeader.values(), 0x0c, localNode.date);
    Bytes::WriteLE<u32>(localNodeHeader.values(), 0x0e, localNode.uncompressedCRC32);
    Bytes::WriteLE<u32>(localNodeHeader.values(), 0x12, localNode.compressedSize);
    Bytes::WriteLE<u32>(localNodeHeader.values(), 0x16, localNode.uncompressedSize);
    u32 localNodePathSize = strlen(localNode.path.values());
    Bytes::WriteLE<u32>(localNodeHeader.values(), 0x1a, localNodePathSize);

    if (!write(localNodeHeader.values(), localNodeHeader.count(), localNodeOffset)) {
        return false;
    }

    if (!write(localNode.path.values(), localNodePathSize,
                localNodeOffset + localNodeHeader.count())) {
        return false;
    }

    return true;
}

bool ZIP::copy(u32 srcOffset, u32 dstOffset, u32 size) {
    if (srcOffset + size < srcOffset) {
        return false;
    }
    if (dstOffset + size < dstOffset) {
        return false;
    }
    while (size > 0) {
        alignas(0x20) Array<u8, 1024> chunk;
        u32 chunkSize = Min<u32>(size, chunk.count());
        if (!read(chunk.values(), chunkSize, srcOffset)) {
            return false;
        }
        if (!write(chunk.values(), chunkSize, dstOffset)) {
            return false;
        }
        srcOffset += chunkSize;
        dstOffset += chunkSize;
        size -= chunkSize;
    }
    return true;
}
