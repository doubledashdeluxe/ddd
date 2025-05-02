#pragma once

#include "portable/Array.hh"

class Random {
public:
    virtual void get(void *data, size_t size);
    virtual u32 get(u32 range);

protected:
    Random();
    ~Random();

    Array<u8, 32 + 256> m_buffer;
    u16 m_offset;
};
