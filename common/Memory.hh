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

private:
    Memory();
};
