#pragma once

#include "portable/Allocator.hh"

class ClientPlatform {
public:
    virtual Allocator &allocator() = 0;
};
