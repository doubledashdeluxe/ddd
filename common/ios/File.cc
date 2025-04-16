#include "File.hh"

#include "common/DCache.hh"
#include "common/Memory.hh"

#include <portable/Array.hh>
#include <portable/Bytes.hh>

extern "C" {
#include <string.h>
}

namespace IOS {

File::File(s32 fd) : Resource(fd) {}

File::File(const char *path, u32 mode) : Resource(path, mode) {}

File::~File() {}

bool File::ok() const {
    return Resource::ok();
}

s32 File::read(void *output, u32 outputSize) {
    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Read;
    request.fd = m_fd;
    request.read.output = Memory::CachedToPhysical(output);
    request.read.outputSize = outputSize;

    Sync(request);

    DCache::Invalidate(output, outputSize);
    return request.result;
}

s32 File::write(const void *input, u32 inputSize) {
    DCache::Flush(input, inputSize);

    alignas(0x20) Request request;
    memset(&request, 0, sizeof(request));
    request.command = Command::Write;
    request.fd = m_fd;
    request.write.input = Memory::CachedToPhysical(input);
    request.write.inputSize = inputSize;

    Sync(request);

    return request.result;
}

bool File::getStats(Stats &stats) {
    alignas(0x20) Array<u8, 0x8> out;

    if (ioctl(Ioctl::GetFileStats, nullptr, 0, out.values(), out.count()) != 0) {
        return false;
    }

    stats.size = Bytes::ReadBE<u32>(out.values(), 0x0);
    stats.offset = Bytes::ReadBE<u32>(out.values(), 0x4);
    return true;
}

} // namespace IOS
