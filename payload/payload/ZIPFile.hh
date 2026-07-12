#pragma once

#include <cube/storage/Storage.hh>
#include <portable/ZIP.hh>

class ZIPFile : public ZIP {
public:
    ZIPFile(Storage::FileHandle &file);
    ~ZIPFile();

    bool read(void *dst, u32 size, u32 offset) override;
    bool write(const void *src, u32 size, u32 offset) override;
    bool truncate(u64 size) override;
    bool size(u64 &size) override;
    u32 getDOSTime() override;

    Storage::FileHandle &m_file;
};

class OwnedZIPFile
    : private Storage::FileHandle
    , public ZIPFile {
public:
    OwnedZIPFile(const char *path);
};
