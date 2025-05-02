#pragma once

#include "portable/Allocator.hh"
#include "portable/network/DNS.hh"

class ClientPlatform {
public:
    virtual Allocator &allocator() = 0;
    virtual DNS &dns() = 0;
};
