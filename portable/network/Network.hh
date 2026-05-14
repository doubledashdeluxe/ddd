#pragma once

#include "portable/network/DNS.hh"

class Network {
public:
    virtual bool isRunning() const = 0;
    virtual const char *name() const = 0;
    virtual u32 address() const = 0;
    virtual DNS::Resolvers resolvers() const = 0;
};
