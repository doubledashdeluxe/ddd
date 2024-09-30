#include "DI.hh"

#include <common/Clock.hh>
#include <common/DCache.hh>
#include <common/Platform.hh>

extern "C" volatile u32 disr;
extern "C" volatile u32 dicvr;
extern "C" volatile u32 dicmdbuf0;
extern "C" volatile u32 dicmdbuf1;
extern "C" volatile u32 dicmdbuf2;
extern "C" volatile u32 dimar;
extern "C" volatile u32 dilength;
extern "C" volatile u32 dicr;
extern "C" volatile u32 diimmbuf;

extern "C" volatile u32 pirc;

extern "C" volatile u32 gpio_out;
extern "C" volatile u32 resets;

bool DI::ReadDiscID() {
    disr = 0x54;
    dicmdbuf0 = 0xa8000040;
    dicmdbuf1 = 0x0;
    dicmdbuf2 = 0x20;
    dimar = 0x80000000 & 0x1fffffff;
    dilength = 0x20;
    dicr = 0x3;
    DCache::Invalidate(reinterpret_cast<void *>(0x80000000), 0x20);

    while (dicr & 0x1) {}

    return !(disr & 0x4);
}

bool DI::Read(void *dst, u32 size, u32 offset) {
    disr = 0x54;
    dicmdbuf0 = 0xa8000000;
    dicmdbuf1 = offset >> 2;
    dicmdbuf2 = size;
    dimar = reinterpret_cast<uintptr_t>(dst) & 0x1fffffff;
    dilength = size;
    dicr = 0x3;
    DCache::Invalidate(dst, size);

    while (dicr & 0x1) {}

    return !(disr & 0x4);
}

bool DI::IsInserted() {
    dicmdbuf0 = 0xe0000000;
    diimmbuf = 0x0;
    dicr = 0x1;

    while (dicr & 0x1) {}

    return !(diimmbuf & 0x1000000);
}

void DI::Reset() {
    if (Platform::IsGameCube()) {
        pirc = (pirc & ~0x4) | 0x1;
        Clock::WaitMicroseconds(12);
        pirc = pirc | 0x5;
    } else {
        gpio_out &= ~0x10;
        resets &= ~0x400;
        Clock::WaitMilliseconds(1);
        resets |= 0x400;
    }
}
