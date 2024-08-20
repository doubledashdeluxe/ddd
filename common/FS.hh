#pragma once

#include "common/ios/Resource.hh"

class FS : private IOS::Resource {
public:
    FS();
    ~FS();
    bool ok() const;

    bool writeFile(const char *path, const void *src, u32 size, u16 mode, u8 attributes = 0);

    bool createDir(const char *path, u16 mode, u8 attributes = 0);
    bool remove(const char *path);
    bool rename(const char *srcPath, const char *dstPath);
    bool createFile(const char *path, u16 mode, u8 attributes = 0);

private:
    bool create(u32 ioctl, const char *path, u16 mode, u8 attributes);

    class Ioctl {
    public:
        enum {
            CreateDir = 0x3,
            Remove = 0x7,
            Rename = 0x8,
            CreateFile = 0x9,
        };

    private:
        Ioctl();
    };
};
