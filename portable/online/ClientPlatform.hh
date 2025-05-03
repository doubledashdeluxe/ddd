#pragma once

#include "portable/Allocator.hh"
#include "portable/Array.hh"
#include "portable/crypto/Random.hh"
#include "portable/network/DNS.hh"

struct ClientPlatform {
    ClientPlatform(Allocator &allocator, Random &random, DNS &dns, const Array<u8, 32> &clientK);

    Allocator &allocator;
    Random &random;
    DNS &dns;
    const Array<u8, 32> &clientK;
};
