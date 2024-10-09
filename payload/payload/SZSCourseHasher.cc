#include "SZSCourseHasher.hh"

bool SZSCourseHasher::Hash(ZIPFile &zipFile, const char *filePath, Array<u8, 32> &hash) {
    SZSCourseHasher hasher(zipFile, filePath);
    return hasher.hash(hash);
}

SZSCourseHasher::SZSCourseHasher(ZIPFile &zipFile, const char *filePath)
    : m_reader(zipFile, filePath) {}

SZSCourseHasher::~SZSCourseHasher() {}

bool SZSCourseHasher::hash(Array<u8, 32> &hash) {
    crypto_blake2b_init(&m_ctx, hash.count());
    if (!SZSReader::read()) {
        return false;
    }

    crypto_blake2b_final(&m_ctx, hash.values());
    return true;
}

bool SZSCourseHasher::setSize(u32 /* size */) {
    return true;
}

bool SZSCourseHasher::read(const u8 *&buffer, u32 &size) {
    return m_reader.read(buffer, size);
}

bool SZSCourseHasher::write(const u8 *buffer, u32 size) {
    crypto_blake2b_update(&m_ctx, buffer, size);
    return true;
}
