#include <common/Arena.hh>

#include <common/Align.hh>

extern "C" u32 arenaLo;
extern "C" u32 arenaHi;

void *MEM1Arena::alloc(size_t size, s32 align) {
    if (align >= 0) {
        arenaLo = AlignUp<u32>(arenaLo, align);
        void *ptr = reinterpret_cast<void *>(arenaLo);
        arenaLo += size;
        return ptr;
    } else {
        arenaHi = AlignDown<u32>(arenaHi, -align);
        void *ptr = reinterpret_cast<void *>(arenaHi);
        arenaHi -= size;
        return ptr;
    }
}
