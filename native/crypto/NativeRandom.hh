#pragma once

#include <portable/crypto/Random.hh>

class NativeRandom final : public Random {
public:
    NativeRandom();
    ~NativeRandom();
};
