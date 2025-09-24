#include "DI.hh"

#include <cube/Clock.hh>
#include <cube/DCache.hh>
#include <cube/Platform.hh>
#include <portable/Align.hh>
#include <portable/Log.hh>

extern "C" {
#include <assert.h>
}

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
    ClearInterrupts();

    // Input: adjusted offset in dicmdbuf1, size in dicmdbuf2, buffer in dimar, size in dilength
    // Output: void
    MakeCommand(0xa8, 0x0, 0x40);
    dicmdbuf1 = 0x0;
    dicmdbuf2 = 0x20;
    dimar = 0x80000000 & 0x1fffffff;
    dilength = 0x20;
    SendCommand(true);

    WaitForCommand();

    DCache::Invalidate(reinterpret_cast<void *>(0x80000000), 0x20);
    return !IsDeviceError();
}

bool DI::Read(void *dst, u32 size, u32 offset) {
    assert(IsAligned(dst, 0x20));
    assert(IsAligned(size, 0x20));
    ClearInterrupts();

    // Input: adjusted offset in dicmdbuf1, size in dicmdbuf2, buffer in dimar, size in dilength
    // Output: void
    MakeCommand(0xa8, 0x0, 0x0);
    dicmdbuf1 = offset >> 2;
    dicmdbuf2 = size;
    dimar = reinterpret_cast<uintptr_t>(dst) & 0x1fffffff;
    dilength = size;
    SendCommand(true);

    WaitForCommand();

    DCache::Invalidate(dst, size);
    return !IsDeviceError();
}

bool DI::IsInserted() {
    // Input: void
    // Output: error code in diimmbuf
    MakeCommand(0xe0, 0x0, 0x0);
    diimmbuf = 0x0;
    SendCommand(false);

    WaitForCommand();

    u8 category = diimmbuf >> 24;
    switch (category) {
    case 5:
        ERROR("DI::IsInserted() called before DI was initialized!");
        return false;
    case 1:
    case 3:
        return false;
    default:
        return true;
    }
}

bool DI::IsInitialized() {
    // Input: void
    // Output: error code in diimmbuf
    MakeCommand(0xe0, 0x0, 0x0);
    diimmbuf = 0x0;
    SendCommand(false);

    WaitForCommand();

    u8 category = diimmbuf >> 24;
    return category != 5;
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

void DI::ClearInterrupts() {
    disr = 0x54;
}

void DI::MakeCommand(u8 command, u8 sub0, u16 sub1) {
    dicmdbuf0 = command << 24 | sub0 << 16 | sub1;
}

void DI::SendCommand(bool dma) {
    dicr = 0x1 | dma << 1;
}

void DI::WaitForCommand() {
    while (dicr & 0x1) {}
}

bool DI::IsDeviceError() {
    return disr & 0x4;
}
