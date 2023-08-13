#include <common/DCache.hh>

extern "C" {
#include <dolphin/OSCache.h>
}

void DCache::Store(const void *start, size_t size) {
    DCStoreRange(const_cast<void *>(start), size);
}

void DCache::Flush(const void *start, size_t size) {
    DCFlushRange(const_cast<void *>(start), size);
}

void DCache::Invalidate(void *start, size_t size) {
    DCInvalidateRange(start, size);
}
