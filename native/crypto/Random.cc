#include "Random.hh"

#include <algorithm>
#include <climits>
#include <random>

void Random::Get(void *data, size_t size) {
    static std::independent_bits_engine<std::random_device, CHAR_BIT, u16> engine;
    std::generate(reinterpret_cast<u8 *>(data), reinterpret_cast<u8 *>(data) + size,
            std::ref(engine));
}

u32 Random::Get(u32 range) {
    range--;
    u32 zero = 0;
    u32 mask = ~zero;
    mask >>= __builtin_clz(range | 1);
    uint32_t x;
    do {
        Get(&x, sizeof(x));
        x &= mask;
    } while (x > range);
    return x;
}
