#pragma once

#include "portable/Algorithm.hh"
#include "portable/Array.hh"

class Random {
public:
    virtual void get(void *data, size_t size);
    virtual u32 get(u32 range);

    template <typename S>
    void shuffle(S &sequence, size_t count) {
        for (size_t i = count - 1; i > 0; i--) {
            size_t j = get(i + 1);
            Swap(sequence[i], sequence[j]);
        }
    }

protected:
    Random();
    ~Random();

    Array<u8, 32 + 256> m_buffer;
    u16 m_offset;
};
