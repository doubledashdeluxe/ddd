#include "SZSCourseLoader.hh"

void *SZSCourseLoader::Load(ZIPFile &zipFile, const char *filePath, JKRHeap *heap, u32 *size) {
    SZSCourseLoader loader(zipFile, filePath, heap, size);
    return loader.load();
}

SZSCourseLoader::SZSCourseLoader(ZIPFile &zipFile, const char *filePath, JKRHeap *heap, u32 *size)
    : m_reader(zipFile, filePath), m_heap(heap), m_size(size), m_offset(0) {}

SZSCourseLoader::~SZSCourseLoader() {}

void *SZSCourseLoader::load() {
    return SZSReader::read() ? m_data.release() : nullptr;
}

bool SZSCourseLoader::setSize(u32 size) {
    if (m_size) {
        *m_size = size;
    }
    m_data.reset(new (m_heap, 0x20) u8[size]);
    return m_data.get();
}

bool SZSCourseLoader::read(const u8 *&buffer, u32 &size) {
    return m_reader.read(buffer, size);
}

bool SZSCourseLoader::write(const u8 *buffer, u32 size) {
    memcpy(m_data.get() + m_offset, buffer, size);
    m_offset += size;
    return true;
}
