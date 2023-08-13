#include <common/ICache.hh>

extern "C" {
#include <dolphin/OSCache.h>
}

void ICache::Invalidate(void *start, size_t size) {
    ICInvalidateRange(start, size);
}
