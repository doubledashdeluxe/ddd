#pragma once

#include "payload/ZIPFile.hh"

extern "C" {
#include <monocypher/monocypher.h>
}
#include <portable/Array.hh>
#include <portable/SZSReader.hh>

class SZSCourseHasher : private SZSReader {
public:
    static bool Hash(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash);

private:
    SZSCourseHasher(ZIPFile &zipFile, const char *filePath);
    ~SZSCourseHasher();
    bool hash(Array<u8, 32> &hash);

    bool setSize(u32 size) override;
    bool read(const u8 *&buffer, u32 &size) override;
    bool write(const u8 *buffer, u32 size) override;

    ZIPFile::Reader m_reader;
    crypto_blake2b_ctx m_ctx;
};
