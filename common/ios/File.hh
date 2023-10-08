#pragma once

#include "common/ios/Resource.hh"

namespace IOS {

class File : private Resource {
public:
    File(s32 fd);
    File(const char *path, u32 mode);
    ~File();
    s32 read(void *output, u32 outputSize);
    s32 write(const void *input, u32 inputSize);
};

} // namespace IOS
