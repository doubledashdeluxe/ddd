#pragma once

#include <common/UniquePtr.hh>
#include <common/storage/Storage.hh>
#include <jsystem/JKRHeap.hh>

class ZIPFile {
public:
    class CompressionMethod {
    public:
        enum {
            Store = 0,
            Deflate = 8,
        };

    private:
        CompressionMethod();
    };

    struct EOCD {
        u16 cdNodeCount;
        u32 cdSize;
        u32 cdOffset;
    };

    struct CDNode {
        u16 compressionMethod;
        u16 time;
        u16 date;
        u32 uncompressedCRC32;
        u32 compressedSize;
        u32 uncompressedSize;
        u32 localNodeOffset;
        UniquePtr<char[]> path;
        u32 nextOffset;
    };

    struct LocalNode {
        u16 compressionMethod;
        u16 time;
        u16 date;
        u32 uncompressedCRC32;
        u32 compressedSize;
        u32 uncompressedSize;
        UniquePtr<char[]> path;
        u32 compressedOffset;
    };

    ZIPFile(const char *path, bool &ok);
    ~ZIPFile();
    const EOCD &eocd() const;
    u8 *readFile(const char *path, JKRHeap *heap, s32 alignment, CDNode &cdNode,
            LocalNode &localNode);
    bool writeFile(const char *path, const void *uncompressed, u32 uncompressedSize, JKRHeap *heap,
            s32 alignment, CDNode &cdNode, LocalNode &localNode);

private:
    enum {
        EOCDHeaderSize = 0x16,
        MaxEOCDCommentSize = 0xffff,
        CDNodeHeaderSize = 0x2e,
        MaxCDNodePathSize = 0xffff,
        LocalNodeHeaderSize = 0x1e,
    };

    class AppendGuard {
    public:
        AppendGuard(ZIPFile &zipFile);
        ~AppendGuard();

    private:
        ZIPFile &m_zipFile;
    };

    bool readEOCD(EOCD &eocd);
    bool readEOCD(u32 eocdOffset, const Array<u8, EOCDHeaderSize> &eocdHeader, EOCD &eocd);
    bool readCDNode(u32 cdNodeOffset, JKRHeap *heap, s32 alignment, CDNode &cdNode);
    bool readLocalNode(u32 localNodeOffset, JKRHeap *heap, s32 alignment, LocalNode &localNode);

    bool writeEOCD(u32 eocdOffset, const EOCD &eocd);
    bool writeCDNode(u32 cdNodeOffset, const CDNode &cdNode);
    bool writeLocalNode(u32 localNodeOffset, const LocalNode &localNode);

    bool copy(u32 srcOffset, u32 dstOffset, u32 size);

    Storage::FileHandle m_file;
    u32 m_fileSize;
    EOCD m_eocd;
};
