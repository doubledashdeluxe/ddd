#include "ZIPFile.hh"

#include "payload/DOSTime.hh"

#include <common/Bytes.hh>
extern "C" {
#include <tinf/tinf.h>

#include <string.h>
}

ZIPFile::ZIPFile(const char *path, bool &ok) : m_file(path, Storage::Mode::ReadWrite) {
    ok = false;

    u64 fileSize;
    if (!m_file.size(fileSize)) {
        return;
    }
    if (fileSize > UINT32_MAX) {
        return;
    }
    m_fileSize = fileSize;

    if (!readEOCD(m_eocd)) {
        return;
    }

    ok = true;
}

ZIPFile::~ZIPFile() {}

const ZIPFile::EOCD &ZIPFile::eocd() const {
    return m_eocd;
}

u8 *ZIPFile::readFile(const char *path, JKRHeap *heap, s32 alignment, CDNode &cdNode,
        LocalNode &localNode) {
    u32 cdNodeIndex, cdNodeOffset;
    for (cdNodeIndex = 0, cdNodeOffset = m_eocd.cdOffset; cdNodeIndex < m_eocd.cdNodeCount;
            cdNodeIndex++, cdNodeOffset = cdNode.nextOffset) {
        if (!readCDNode(cdNodeOffset, heap, -alignment, cdNode)) {
            return nullptr;
        }

        if (path[0] == '/') {
            if (!strcmp(cdNode.path.get(), path + 1)) {
                break;
            }
        } else {
            char *cdNodeName = strrchr(cdNode.path.get(), '/');
            cdNodeName = cdNodeName ? cdNodeName + 1 : cdNode.path.get();
            if (!strcmp(cdNodeName, path)) {
                break;
            }
        }
    }
    if (cdNodeIndex == m_eocd.cdNodeCount) {
        return nullptr;
    }

    if (!readLocalNode(cdNode.localNodeOffset, heap, -alignment, localNode)) {
        return nullptr;
    }

    UniquePtr<u8[]> uncompressed(new (heap, alignment) u8[cdNode.uncompressedSize]);
    if (!uncompressed.get()) {
        return nullptr;
    }
    if (cdNode.compressionMethod == CompressionMethod::Store) {
        u32 uncompressedOffset = localNode.compressedOffset;
        if (!m_file.read(uncompressed.get(), cdNode.uncompressedSize, uncompressedOffset)) {
            return nullptr;
        }
    } else if (cdNode.compressionMethod == CompressionMethod::Deflate) {
        UniquePtr<u8[]> compressed(new (heap, -alignment) u8[cdNode.compressedSize]);
        if (!compressed.get()) {
            return nullptr;
        }
        if (!m_file.read(compressed.get(), cdNode.compressedSize, localNode.compressedOffset)) {
            return nullptr;
        }

        unsigned int uncompressedSize = cdNode.uncompressedSize;
        if (tinf_uncompress(uncompressed.get(), &uncompressedSize, compressed.get(),
                    cdNode.compressedSize) != TINF_OK) {
            return nullptr;
        }
        if (uncompressedSize != cdNode.uncompressedSize) {
            return nullptr;
        }
    } else {
        return nullptr;
    }

    if (tinf_crc32(uncompressed.get(), cdNode.uncompressedSize) != cdNode.uncompressedCRC32) {
        return nullptr;
    }
    return uncompressed.release();
}

bool ZIPFile::writeFile(const char *path, const void *uncompressed, u32 uncompressedSize,
        JKRHeap *heap, s32 alignment, CDNode &cdNode, LocalNode &localNode) {
    if (path[0] != '/') {
        return false;
    }

    EOCD eocd;
    eocd.cdNodeCount = m_eocd.cdNodeCount + 1;
    if (eocd.cdNodeCount < m_eocd.cdNodeCount) {
        return false;
    }

    u32 cdNodeIndex, cdNodeOffset;
    for (cdNodeIndex = 0, cdNodeOffset = m_eocd.cdOffset; cdNodeIndex < m_eocd.cdNodeCount;
            cdNodeIndex++, cdNodeOffset = cdNode.nextOffset) {
        if (!readCDNode(cdNodeOffset, heap, -alignment, cdNode)) {
            return false;
        }

        if (!strcmp(cdNode.path.get(), path + 1)) {
            break;
        }
    }
    bool isNew = cdNodeIndex == m_eocd.cdNodeCount;

    AppendGuard appendGuard(*this);

    u32 offset = m_fileSize;
    localNode.compressionMethod = CompressionMethod::Store;
    u32 dosTime = DOSTime::Now();
    localNode.time = dosTime >> 0;
    localNode.date = dosTime >> 16;
    localNode.uncompressedCRC32 = tinf_crc32(uncompressed, uncompressedSize);
    localNode.compressedSize = uncompressedSize;
    localNode.uncompressedSize = uncompressedSize;
    u32 pathLength = strlen(path + 1);
    localNode.path.reset(new (heap, -alignment) char[pathLength + 1]);
    if (!localNode.path.get()) {
        return false;
    }
    localNode.path.get()[pathLength] = '\0';
    memcpy(localNode.path.get(), path + 1, pathLength);
    localNode.compressedOffset = offset + LocalNodeHeaderSize + pathLength;
    if (offset + LocalNodeHeaderSize < offset) {
        return false;
    }
    if (localNode.compressedOffset < offset) {
        return false;
    }
    if (!writeLocalNode(offset, localNode)) {
        return false;
    }
    offset = localNode.compressedOffset;

    if (offset + uncompressedSize < offset) {
        return false;
    }
    if (!m_file.write(uncompressed, uncompressedSize, offset)) {
        return false;
    }
    offset += uncompressedSize;
    eocd.cdOffset = offset;

    if (isNew) {
        if (!copy(m_eocd.cdOffset, offset, m_eocd.cdSize)) {
            return false;
        }
        offset += m_eocd.cdSize;
    } else {
        if (!copy(m_eocd.cdOffset, offset, cdNodeOffset - m_eocd.cdOffset)) {
            return false;
        }
        offset += cdNodeOffset - m_eocd.cdOffset;
        if (!copy(cdNode.nextOffset, offset, m_eocd.cdSize - (cdNodeOffset - m_eocd.cdOffset))) {
            return false;
        }
        offset += m_eocd.cdSize - (cdNodeOffset - m_eocd.cdOffset);
    }

    cdNode.compressionMethod = localNode.compressionMethod;
    cdNode.time = localNode.time;
    cdNode.date = localNode.date;
    cdNode.uncompressedCRC32 = localNode.uncompressedCRC32;
    cdNode.compressedSize = localNode.compressedSize;
    cdNode.uncompressedSize = localNode.uncompressedSize;
    cdNode.localNodeOffset = m_fileSize;
    cdNode.path.reset(new (heap, -alignment) char[pathLength + 1]);
    if (!cdNode.path.get()) {
        return false;
    }
    cdNode.path.get()[pathLength] = '\0';
    memcpy(cdNode.path.get(), path + 1, pathLength);
    cdNode.nextOffset = offset + CDNodeHeaderSize + pathLength;
    if (offset + CDNodeHeaderSize < offset) {
        return false;
    }
    if (cdNode.nextOffset < offset) {
        return false;
    }
    if (!writeCDNode(offset, cdNode)) {
        return false;
    }
    offset = cdNode.nextOffset;
    eocd.cdSize = offset - eocd.cdOffset;

    if (offset + EOCDHeaderSize < offset) {
        return false;
    }
    if (!writeEOCD(offset, eocd)) {
        return false;
    }
    offset += EOCDHeaderSize;

    m_fileSize = offset;
    m_eocd = eocd;
    return true;
}

ZIPFile::AppendGuard::AppendGuard(ZIPFile &zipFile) : m_zipFile(zipFile) {}

ZIPFile::AppendGuard::~AppendGuard() {
    m_zipFile.m_file.truncate(m_zipFile.m_fileSize);
}

bool ZIPFile::readEOCD(EOCD &eocd) {
    if (m_fileSize < EOCDHeaderSize) {
        return false;
    }

    Array<u8, EOCDHeaderSize> eocdHeader;
    u32 minEOCDOffset = m_fileSize > MaxEOCDCommentSize ? m_fileSize - MaxEOCDCommentSize : 0;
    u32 maxEOCDOffset = m_fileSize - EOCDHeaderSize;
    u32 eocdOffset = maxEOCDOffset;
    if (!m_file.read(eocdHeader.values(), eocdHeader.count(), eocdOffset)) {
        return false;
    }
    while (!readEOCD(eocdOffset, eocdHeader, eocd)) {
        if (eocdOffset == minEOCDOffset) {
            return false;
        }

        eocdOffset--;
        eocdHeader.rotateRight(1);
        if (!m_file.read(eocdHeader.values(), 1, eocdOffset)) {
            return false;
        }
    }

    return true;
}

bool ZIPFile::readEOCD(u32 eocdOffset, const Array<u8, EOCDHeaderSize> &eocdHeader, EOCD &eocd) {
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

bool ZIPFile::readCDNode(u32 cdNodeOffset, JKRHeap *heap, s32 alignment, CDNode &cdNode) {
    Array<u8, CDNodeHeaderSize> cdNodeHeader;
    if (!m_file.read(cdNodeHeader.values(), cdNodeHeader.count(), cdNodeOffset)) {
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

    cdNode.path.reset(new (heap, alignment) char[cdNodePathSize + 1]);
    if (!cdNode.path.get()) {
        return false;
    }
    cdNode.path.get()[cdNodePathSize] = '\0';
    if (!m_file.read(cdNode.path.get(), cdNodePathSize, cdNodePathOffset)) {
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

bool ZIPFile::readLocalNode(u32 localNodeOffset, JKRHeap *heap, s32 alignment,
        LocalNode &localNode) {
    Array<u8, LocalNodeHeaderSize> localNodeHeader;
    if (!m_file.read(localNodeHeader.values(), localNodeHeader.count(), localNodeOffset)) {
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

    localNode.path.reset(new (heap, alignment) char[localNodePathSize + 1]);
    if (!localNode.path.get()) {
        return false;
    }
    localNode.path.get()[localNodePathSize] = '\0';
    if (!m_file.read(localNode.path.get(), localNodePathSize, localNodePathOffset)) {
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

bool ZIPFile::writeEOCD(u32 eocdOffset, const EOCD &eocd) {
    Array<u8, EOCDHeaderSize> eocdHeader;
    memset(eocdHeader.values(), 0, eocdHeader.count());
    u32 eocdSignature;
    memcpy(&eocdSignature, "PK\5\6", sizeof(eocdSignature));
    memcpy(&eocdHeader[0x00], &eocdSignature, sizeof(eocdSignature));
    Bytes::WriteLE<u16>(eocdHeader.values(), 0x08, eocd.cdNodeCount);
    Bytes::WriteLE<u16>(eocdHeader.values(), 0x0a, eocd.cdNodeCount);
    Bytes::WriteLE<u32>(eocdHeader.values(), 0x0c, eocd.cdSize);
    Bytes::WriteLE<u32>(eocdHeader.values(), 0x10, eocd.cdOffset);

    if (!m_file.write(eocdHeader.values(), eocdHeader.count(), eocdOffset)) {
        return false;
    }

    return true;
}

bool ZIPFile::writeCDNode(u32 cdNodeOffset, const CDNode &cdNode) {
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
    u32 cdNodePathSize = strlen(cdNode.path.get());
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x1c, cdNodePathSize);
    Bytes::WriteLE<u32>(cdNodeHeader.values(), 0x2a, cdNode.localNodeOffset);

    if (!m_file.write(cdNodeHeader.values(), cdNodeHeader.count(), cdNodeOffset)) {
        return false;
    }

    if (!m_file.write(cdNode.path.get(), cdNodePathSize, cdNodeOffset + cdNodeHeader.count())) {
        return false;
    }

    return true;
}

bool ZIPFile::writeLocalNode(u32 localNodeOffset, const LocalNode &localNode) {
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
    u32 localNodePathSize = strlen(localNode.path.get());
    Bytes::WriteLE<u32>(localNodeHeader.values(), 0x1a, localNodePathSize);

    if (!m_file.write(localNodeHeader.values(), localNodeHeader.count(), localNodeOffset)) {
        return false;
    }

    if (!m_file.write(localNode.path.get(), localNodePathSize,
                localNodeOffset + localNodeHeader.count())) {
        return false;
    }

    return true;
}

bool ZIPFile::copy(u32 srcOffset, u32 dstOffset, u32 size) {
    if (srcOffset + size < srcOffset) {
        return false;
    }
    if (dstOffset + size < dstOffset) {
        return false;
    }
    while (size > 0) {
        Array<u8, 512> buffer;
        u32 chunkSize = Min<u32>(size, buffer.count());
        if (!m_file.read(buffer.values(), chunkSize, srcOffset)) {
            return false;
        }
        if (!m_file.write(buffer.values(), chunkSize, dstOffset)) {
            return false;
        }
        srcOffset += chunkSize;
        dstOffset += chunkSize;
        size -= chunkSize;
    }
    return true;
}
