#pragma once

#include <portable/Allocator.hh>
#include <portable/Array.hh>
#include <portable/crypto/Random.hh>

#include <vector>

struct ServerPlatform {
    Allocator &allocator;
    Random &random;
    const Array<u8, 32> &serverK;
    const std::vector<u8> &data;
};
