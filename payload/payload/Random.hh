#pragma once

#include "payload/Mutex.hh"

#include <common/Array.hh>

class Random {
public:
    static void Init();
    static void Get(void *data, size_t size);

private:
    static bool s_isInit;
    static Mutex *s_mutex;
    static Array<u8, 32 + 256> s_buffer;
    static u16 s_offset;
};
