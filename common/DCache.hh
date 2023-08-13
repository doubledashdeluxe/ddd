#pragma once

#include "common/Types.hh"

class DCache {
public:
    static void Store(const void *start, size_t size);
    static void Flush(const void *start, size_t size);
    static void Invalidate(void *start, size_t size);

private:
    DCache();
};
