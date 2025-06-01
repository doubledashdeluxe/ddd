extern "C" {
#include "VI.h"
}

#include <cube/Console.hh>
#include <cube/Memory.hh>
#include <cube/VI.hh>
#include <payload/Lock.hh>

extern "C" volatile u16 visel;

void VIInit() {
    REPLACED(VIInit)();
    REPLACED(VISetNextFrameBuffer)(VI::Instance()->getXFB());
    VISetBlack(false);
}

void VISetNextFrameBuffer(void *fb) {
    fb = reinterpret_cast<void *>(Memory::CachedToPhysical(fb));
    REPLACED(VISetNextFrameBuffer(fb));
}

void VISetBlack(BOOL black) {
    if (Console::Instance()->m_isActive) {
        black = false;
    }
    REPLACED(VISetBlack)(black);
}

u32 VIGetDTVStatus() {
    Lock<NoInterrupts> lock;
    return visel & 1;
}
