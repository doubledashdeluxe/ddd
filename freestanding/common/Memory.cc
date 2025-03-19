#include <common/Memory.hh>

uintptr_t Memory::CachedToPhysical(uintptr_t addr) {
    return addr & 0x7fffffff;
}

uintptr_t Memory::PhysicalToCached(uintptr_t addr) {
    return addr | 0x80000000;
}
