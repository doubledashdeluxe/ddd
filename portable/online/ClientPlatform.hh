#pragma once

#include "portable/Allocator.hh"
#include "portable/Array.hh"
#include "portable/crypto/Random.hh"
#include "portable/network/DNS.hh"

class ClientPlatform {
public:
    virtual Allocator &allocator() = 0;
    virtual Random &random() = 0;
    virtual DNS &dns() = 0;
    virtual const Array<u8, 32> &clientK() = 0;
};
