#pragma once

#include "common/Types.hh"

class ICache {
public:
    static void Invalidate(void *start, size_t size);

private:
    ICache();
};
