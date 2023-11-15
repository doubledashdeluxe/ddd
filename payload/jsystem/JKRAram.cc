#include "JKRAram.hh"

extern "C" {
#include <dolphin/AR.h>
#include <dolphin/ARQ.h>
}

JKRAram::JKRAram(u32 audioBufSize, u32 /* graphBufSize */, s32 priority)
    : JKRThread(0x4000, 0x10, priority) {
    ARInit(&m_stackIndex, 1);
    ARQInit();
    m_audioBuf = ARAlloc(audioBufSize);
    m_audioBufSize = audioBufSize;
    m_graphBuf = 0;
    m_graphBufSize = 0;
    m_userBuf = 0;
    m_userBufSize = 0;
    m_heap = new (JKRHeap::GetSystemHeap(), 0x4) JKRAramHeap(0, 0);
}
