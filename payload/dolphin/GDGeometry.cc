extern "C" {
#include "GDGeometry.h"
}

#include <common/Memory.hh>

void GDSetArray(u32 attr, const void *base, u8 stride) {
    REPLACED(GDSetArrayRaw)(attr, Memory::CachedToPhysical(base), stride);
}

void GDSetArrayRaw(u32 attr, u32 base, u8 stride) {
    base = Memory::CachedToPhysical(reinterpret_cast<void *>(base | 0x80000000));
    REPLACED(GDSetArrayRaw)(attr, base, stride);
}
