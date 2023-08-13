#include <common/DCache.hh>

#include <common/Align.hh>

void DCache::Store(const void *start, size_t size) {
    if (size == 0) {
        return;
    }
    size_t address = reinterpret_cast<size_t>(start);
    size = AlignUp(size, 0x20);
    do {
        asm("dcbst 0, %0" : : "rm"(address));
        address += 0x20;
        size -= 0x20;
    } while (size > 0);
    asm("sync");
}

void DCache::Flush(const void *start, size_t size) {
    if (size == 0) {
        return;
    }
    size_t address = reinterpret_cast<size_t>(start);
    size = AlignUp(size, 0x20);
    do {
        asm("dcbf 0, %0" : : "rm"(address));
        address += 0x20;
        size -= 0x20;
    } while (size > 0);
    asm("sync");
}

void DCache::Invalidate(void *start, size_t size) {
    if (size == 0) {
        return;
    }
    size_t address = reinterpret_cast<size_t>(start);
    size = AlignUp(size, 0x20);
    do {
        asm("dcbi 0, %0" : : "rm"(address));
        address += 0x20;
        size -= 0x20;
    } while (size > 0);
}
