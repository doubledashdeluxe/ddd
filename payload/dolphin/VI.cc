extern "C" {
#include "VI.h"
}

#include <common/Console.hh>
#include <common/VI.hh>
#include <payload/Lock.hh>

extern "C" volatile u16 visel;

extern "C" void VIInit() {
    REPLACED(VIInit)();
    VISetNextFrameBuffer(VI::Instance()->getXFB());
    VISetBlack(false);
}

extern "C" void VISetNextFrameBuffer(void *fb) {
    if (Console::Instance()->m_isActive) {
        fb = VI::Instance()->getXFB();
    }
    REPLACED(VISetNextFrameBuffer)(fb);
}

extern "C" void VISetBlack(BOOL black) {
    if (Console::Instance()->m_isActive) {
        black = false;
    }
    REPLACED(VISetBlack)(black);
}

extern "C" u32 VIGetDTVStatus() {
    Lock<NoInterrupts> lock;
    return visel & 1;
}
