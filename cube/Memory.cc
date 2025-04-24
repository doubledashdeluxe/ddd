#include "Memory.hh"

uintptr_t Memory::UncachedToPhysical(uintptr_t addr) {
    return addr & 0x3fffffff;
}

uintptr_t Memory::PhysicalToUncached(uintptr_t addr) {
    return addr | 0xc0000000;
}
