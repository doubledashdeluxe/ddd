#include "NativeRandom.hh"

#include <algorithm>
#include <climits>
#include <random>

NativeRandom::NativeRandom() {
    static std::independent_bits_engine<std::random_device, CHAR_BIT, u16> engine;
    std::generate(m_buffer.values(), m_buffer.values() + 32, std::ref(engine));
}

NativeRandom::~NativeRandom() = default;
