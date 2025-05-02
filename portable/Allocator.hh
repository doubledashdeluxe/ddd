#pragma once

#include "portable/Types.hh"

class Allocator {
public:
    virtual void *alloc(size_t size) = 0;
};

void *operator new(size_t size, Allocator &allocator);
