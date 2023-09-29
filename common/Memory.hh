#pragma once

#include "common/Types.hh"

class Memory {
public:
    template <typename T>
    static uintptr_t VirtualToPhysical(T *ptr) {
        return reinterpret_cast<uintptr_t>(ptr) & 0x7fffffff;
    }

    template <typename T>
    static T *PhysicalToVirtual(uintptr_t addr) {
        return reinterpret_cast<T *>(addr | 0x80000000);
    }

    template <typename T>
    static bool IsMEM1(T *ptr) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return addr >= 0x80000000 && addr < 0x81800000 || addr >= 0xc0000000 && addr < 0xc1800000;
    }

    template <typename T>
    static bool IsMEM2(T *ptr) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return addr >= 0x90000000 && addr < 0x94000000 || addr >= 0xd0000000 && addr < 0xd4000000;
    }

    template <typename T>
    static bool IsAligned(T *ptr, size_t alignment) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        return addr % alignment == 0;
    }

private:
    Memory();
};
