#pragma once

#include "portable/Types.hh"

class Network {
public:
    virtual bool isRunning() const = 0;
    virtual const char *name() const = 0;
    virtual u32 address() const = 0;
};
