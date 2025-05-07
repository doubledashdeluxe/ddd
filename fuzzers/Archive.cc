#include <portable/Align.hh>
#include <portable/Archive.hh>

#include <algorithm>
#include <cstdlib>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
#ifdef _WIN32
    u8 *zipData = static_cast<u8 *>(_aligned_malloc(0x20, AlignUp(size, 0x20)));
#else
    u8 *zipData = static_cast<u8 *>(aligned_alloc(0x20, AlignUp(size, 0x20)));
#endif
    memcpy(zipData, data, size);
    for (size_t i = 0; i < std::min<size_t>(size, 0x20); i++) {
        Archive archive(zipData + i);
        archive.isValid(size - i);
    }
#ifdef _WIN32
    _aligned_free(zipData);
#else
    free(zipData);
#endif
    return 0;
}
