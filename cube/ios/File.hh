#pragma once

#include "cube/ios/Resource.hh"

namespace IOS {

class File : private Resource {
public:
    struct Stats {
        u32 size;
        u32 offset;
    };

    File(s32 fd);
    File(const char *path, u32 mode);
    ~File();
    bool ok() const;
    s32 read(void *output, u32 outputSize);
    s32 write(const void *input, u32 inputSize);
    bool getStats(Stats &stats);

private:
    class Ioctl {
    public:
        enum {
            GetFileStats = 0xb,
        };

    private:
        Ioctl();
    };
};

} // namespace IOS
