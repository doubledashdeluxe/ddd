#pragma once

#include "portable/Array.hh"

extern "C" {
#include <miniz/miniz.h>
}

class ZIP {
public:
    enum {
        PathSize = 128,
    };

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
        Array<char, 128> path;
        u32 nextOffset;
    };

    struct LocalNode {
        u16 compressionMethod;
        u16 time;
        u16 date;
        u32 uncompressedCRC32;
        u32 compressedSize;
        u32 uncompressedSize;
        Array<char, 128> path;
        u32 compressedOffset;
    };

    class Reader {
    public:
        Reader(ZIP &zip, const char *path);
        ~Reader();
        bool ok() const;
        const CDNode *cdNode() const;
        const LocalNode *localNode() const;
        const u32 *size() const;
        bool read(const u8 *&buffer, u32 &size);

    private:
        bool m_ok;
        ZIP &m_zip;
        CDNode m_cdNode;
        LocalNode m_localNode;
        tinfl_decompressor m_decompressor;
        tinfl_status m_status;
        u32 m_compressedOffset;
        u32 m_uncompressedOffset;
        u32 m_crc32;
        alignas(0x20) Array<u8, 4 * 1024> m_compressedBuffer;
        alignas(0x20) Array<u8, TINFL_LZ_DICT_SIZE> m_uncompressedBuffer;
    };

    class Writer {
    public:
        Writer(ZIP &zip, const char *path, u32 size);
        ~Writer();
        bool ok() const;
        const CDNode *cdNode() const;
        const LocalNode *localNode() const;
        bool write(const u8 *buffer, u32 size);

    private:
        bool m_ok;
        ZIP &m_zip;
        CDNode m_cdNode;
        LocalNode m_localNode;
        u32 m_size;
        EOCD m_eocd;
        bool m_isNew;
        u32 m_cdNodeOffset;
        u32 m_offset;
    };

    ZIP();
    ~ZIP();
    bool ok() const;
    const EOCD *eocd() const;

protected:
    void setup();

private:
    enum {
        EOCDHeaderSize = 0x16,
        MaxEOCDCommentSize = 0xffff,
        CDNodeHeaderSize = 0x2e,
        MaxCDNodePathSize = 0xffff,
        LocalNodeHeaderSize = 0x1e,
    };

    virtual bool read(void *dst, u32 size, u32 offset) = 0;
    virtual bool write(const void *src, u32 size, u32 offset) = 0;
    virtual bool truncate(u64 size) = 0;
    virtual bool size(u64 &size) = 0;
    virtual u32 getDOSTime() = 0;

    bool readEOCD(EOCD &eocd);
    bool readEOCD(u32 eocdOffset, const Array<u8, EOCDHeaderSize> &eocdHeader, EOCD &eocd);
    bool readCDNode(u32 cdNodeOffset, CDNode &cdNode);
    bool readLocalNode(u32 localNodeOffset, LocalNode &localNode);

    bool writeEOCD(u32 eocdOffset, const EOCD &eocd);
    bool writeCDNode(u32 cdNodeOffset, const CDNode &cdNode);
    bool writeLocalNode(u32 localNodeOffset, const LocalNode &localNode);

    bool copy(u32 srcOffset, u32 dstOffset, u32 size);

    bool m_ok;
    u32 m_fileSize;
    EOCD m_eocd;
};
