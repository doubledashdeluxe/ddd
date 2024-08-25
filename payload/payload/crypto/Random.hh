#pragma once

#ifdef __CWCC__
#include "payload/Mutex.hh"

#include <common/Array.hh>
#else
#include <common/Types.hh>
#endif

class Random {
public:
    static void Init();
    static void Get(void *data, size_t size);

private:
    Random();

#ifdef __CWCC__
    static bool s_isInit;
    static Mutex *s_mutex;
    static Array<u8, 32 + 256> s_buffer;
    static u16 s_offset;
#endif
};
