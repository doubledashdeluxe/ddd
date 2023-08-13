#include <common/ICache.hh>

#include <common/Align.hh>

void ICache::Invalidate(void *start, size_t size) {
    if (size == 0) {
        return;
    }
    size_t address = reinterpret_cast<size_t>(start);
    size = AlignUp(size, 0x20);
    do {
        asm("icbi 0, %0" : : "m"(address));
        address += 0x20;
        size -= 0x20;
    } while (size > 0);
    asm("sync; isync");
}
