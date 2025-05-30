#pragma once

#include "portable/Types.hh"

template <typename T>
bool IsAligned(T val, size_t alignment) {
    return val % alignment == 0;
}

template <typename T>
static bool IsAligned(T *ptr, size_t alignment) {
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    return addr % alignment == 0;
}

template <typename T>
T AlignDown(T val, size_t alignment) {
    return val / alignment * alignment;
}

template <typename T>
T AlignUp(T val, size_t alignment) {
    return AlignDown<T>(val + alignment - 1, alignment);
}
