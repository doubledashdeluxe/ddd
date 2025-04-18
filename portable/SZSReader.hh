#pragma once

#include "portable/Array.hh"

class SZSReader {
public:
    SZSReader();
    ~SZSReader();
    bool read();

private:
    virtual bool setSize(u32 size) = 0;
    virtual bool read(const u8 *&buffer, u32 &size) = 0;
    virtual bool write(const u8 *buffer, u32 size) = 0;

    bool read(u8 &value);
    bool write(u8 value);

    const u8 *m_compressedBuffer;
    u32 m_compressedBufferSize;
    Array<u8, 4096> m_uncompressedBuffer;
    u32 m_uncompressedBufferOffset;
};
