#include "ZIPFile.hh"

#include "payload/DOSTime.hh"

ZIPFile::ZIPFile(const char *path) : m_file(path, Storage::Mode::ReadWrite) {
    setup();
}

ZIPFile::~ZIPFile() {}

bool ZIPFile::read(void *dst, u32 size, u32 offset) {
    return m_file.read(dst, size, offset);
}

bool ZIPFile::write(const void *src, u32 size, u32 offset) {
    return m_file.write(src, size, offset);
}

bool ZIPFile::truncate(u64 size) {
    return m_file.truncate(size);
}

bool ZIPFile::size(u64 &size) {
    return m_file.size(size);
}

u32 ZIPFile::getDOSTime() {
    return DOSTime::Now();
}
