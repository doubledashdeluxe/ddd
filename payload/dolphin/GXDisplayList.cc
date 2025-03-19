extern "C" {
#include "GXDisplayList.h"
}

#include <common/Memory.hh>

void GXCallDisplayList(const void *data, u32 size) {
    data = reinterpret_cast<void *>(Memory::CachedToPhysical(data) | 0x80000000);
    REPLACED(GXCallDisplayList)(data, size);
}
