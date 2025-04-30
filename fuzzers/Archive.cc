#include <portable/Align.hh>
#include <portable/Archive.hh>

#include <algorithm>
#include <cstdlib>
#include <cstring>

extern "C" int LLVMFuzzerTestOneInput(const u8 *data, size_t size) {
    u8 *zipData = static_cast<u8 *>(aligned_alloc(0x20, AlignUp(size, 0x20)));
    memcpy(zipData, data, size);
    for (size_t i = 0; i < std::min<size_t>(size, 0x20); i++) {
        Archive archive(zipData + i);
        archive.isValid(size - i);
    }
    free(zipData);
    return 0;
}
