#include "Random.hh"

#include "portable/Algorithm.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <string.h>
}

Random::Random() : m_offset(m_buffer.count()) {}

Random::~Random() {}

void Random::get(void *data, size_t size) {
    while (size > 0) {
        if (m_offset == m_buffer.count()) {
            Array<u8, 8> nonce(0);
            crypto_chacha20_djb(m_buffer.values(), nullptr, m_buffer.count(), m_buffer.values(),
                    nonce.values(), 0);
            m_offset = 32;
        }

        size_t chunkSize = Min(size, m_buffer.count() - m_offset);
        memcpy(data, m_buffer.values() + m_offset, chunkSize);
        data = static_cast<u8 *>(data) + chunkSize;
        size -= chunkSize;
        m_offset += chunkSize;
    }

    crypto_wipe(m_buffer.values() + 32, m_offset - 32);
}

u32 Random::get(u32 range) {
    range--;
    u32 zero = 0;
    u32 mask = ~zero;
#ifdef __CWCC__
    mask >>= __cntlzw(range | 1);
#else
    mask >>= __builtin_clz(range | 1);
#endif
    uint32_t x;
    do {
        get(&x, sizeof(x));
        x &= mask;
    } while (x > range);
    return x;
}
