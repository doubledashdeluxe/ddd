#pragma once

#include "payload/SZSReader.hh"
#include "payload/ZIPFile.hh"

#include <jsystem/JKRHeap.hh>
#include <portable/UniquePtr.hh>

class SZSCourseLoader : private SZSReader {
public:
    static void *Load(ZIPFile &zipFile, const char *filePath, JKRHeap *heap, u32 *size = nullptr);

private:
    SZSCourseLoader(ZIPFile &zipFile, const char *filePath, JKRHeap *heap, u32 *size);
    ~SZSCourseLoader();
    void *load();

    bool setSize(u32 size) override;
    bool read(const u8 *&buffer, u32 &size) override;
    bool write(const u8 *buffer, u32 size) override;

    ZIPFile::Reader m_reader;
    JKRHeap *m_heap;
    u32 *m_size;
    UniquePtr<u8[]> m_data;
    u32 m_offset;
};
