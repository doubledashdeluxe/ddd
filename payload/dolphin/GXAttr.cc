extern "C" {
#include "GXAttr.h"
}

#include <common/Memory.hh>

void GXSetArray(u32 attr, const void *base, u8 stride) {
    base = reinterpret_cast<void *>(Memory::CachedToPhysical(base));
    REPLACED(GXSetArray)(attr, base, stride);
}
