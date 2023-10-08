#include "File.hh"

#include "common/DCache.hh"
#include "common/Memory.hh"

extern "C" {
#include <string.h>
}

namespace IOS {

File::File(s32 fd) : Resource(fd) {}

File::File(const char *path, u32 mode) : Resource(path, mode) {}

File::~File() {}

s32 File::read(void *output, u32 outputSize) {
    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Read;
    request.fd = m_fd;
    request.read.output = Memory::VirtualToPhysical(output);
    request.read.outputSize = outputSize;

    Sync(request);

    DCache::Invalidate(output, outputSize);
    return request.result;
}

s32 File::write(const void *input, u32 inputSize) {
    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Write;
    request.fd = m_fd;
    request.write.input = Memory::VirtualToPhysical(input);
    request.write.inputSize = inputSize;

    Sync(request);

    return request.result;
}

} // namespace IOS
