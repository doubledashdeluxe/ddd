#include <common/Memory.hh>

uintptr_t Memory::CachedToPhysical(uintptr_t addr) {
    if (addr >= 0x80000000 && addr < 0x80400000) {
        addr += 0x400000;
    }
    return addr & 0x7fffffff;
}

uintptr_t Memory::PhysicalToCached(uintptr_t addr) {
    if (addr >= 0x00400000 && addr < 0x00800000) {
        addr -= 0x400000;
    }
    return addr | 0x80000000;
}
