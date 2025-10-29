#pragma once

#include <portable/Allocator.hh>
#include <portable/crypto/Random.hh>
#include <portable/crypto/Types.hh>

#include <vector>

struct ServerPlatform {
    Allocator &allocator;
    Random &random;
    const Key &serverK;
    const std::vector<u8> &data;
};
