#pragma once

#include <portable/Types.hh>

class Random {
public:
    Random() = delete;

    static void Init();
    static void Get(void *data, size_t size);
    static u32 Get(u32 range);
};
