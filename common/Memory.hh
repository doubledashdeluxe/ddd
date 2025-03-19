#pragma once

#include "common/Types.hh"

class Memory {
public:
    template <typename T>
    static uintptr_t CachedToPhysical(T *ptr) {
        return CachedToPhysical(reinterpret_cast<uintptr_t>(ptr));
    }

    template <typename T>
    static T *PhysicalToCached(uintptr_t addr) {
        return reinterpret_cast<T *>(PhysicalToCached(addr));
    }

    template <typename T>
    static uintptr_t UncachedToPhysical(T *ptr) {
        return UncachedToPhysical(reinterpret_cast<uintptr_t>(ptr));
    }

    template <typename T>
    static T *PhysicalToUncached(uintptr_t addr) {
        return reinterpret_cast<T *>(PhysicalToUncached(addr));
    }

    template <typename T>
    static bool IsMEM1(T *ptr) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return (addr >= 0x80000000 && addr < 0x81800000) ||
                (addr >= 0xc0000000 && addr < 0xc1800000);
    }

    template <typename T>
    static bool IsMEM2(T *ptr) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return (addr >= 0x90000000 && addr < 0x94000000) ||
                (addr >= 0xd0000000 && addr < 0xd4000000);
    }

    template <typename T>
    static bool IsAligned(T *ptr, size_t alignment) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return addr % alignment == 0;
    }

private:
    static uintptr_t CachedToPhysical(uintptr_t addr);
    static uintptr_t PhysicalToCached(uintptr_t addr);
    static uintptr_t UncachedToPhysical(uintptr_t addr);
    static uintptr_t PhysicalToUncached(uintptr_t addr);

    Memory();
};
