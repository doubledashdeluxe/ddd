#pragma once

#include "payload/ZIPFile.hh"

extern "C" {
#include <monocypher/monocypher.h>
}
#include <portable/SZSReader.hh>
#include <portable/crypto/Types.hh>

class SZSCourseHasher : private SZSReader {
public:
    static bool Hash(ZIPFile &zipFile, const char *filePath, ::Hash &hash);

private:
    SZSCourseHasher(ZIPFile &zipFile, const char *filePath);
    ~SZSCourseHasher();
    bool hash(::Hash &hash);

    bool setSize(u32 size) override;
    bool read(const u8 *&buffer, u32 &size) override;
    bool write(const u8 *buffer, u32 size) override;

    ZIPFile::Reader m_reader;
    crypto_blake2b_ctx m_ctx;
};
