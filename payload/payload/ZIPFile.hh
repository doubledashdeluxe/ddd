#pragma once

#include <common/storage/Storage.hh>
#include <portable/ZIP.hh>

class ZIPFile : public ZIP {
public:
    ZIPFile(const char *path);
    ~ZIPFile();

private:
    bool read(void *dst, u32 size, u32 offset) override;
    bool write(const void *src, u32 size, u32 offset) override;
    bool truncate(u64 size) override;
    bool size(u64 &size) override;
    u32 getDOSTime() override;

    Storage::FileHandle m_file;
};
