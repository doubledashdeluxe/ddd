#pragma once

#include "jsystem/JKRAramBlock.hh"
#include "jsystem/JKRAramHeap.hh"
#include "jsystem/JKRHeap.hh"
#include "jsystem/JKRThread.hh"

#include <payload/Replace.hh>

class JKRAram : public JKRThread {
public:
    static JKRAramHeap *GetHeap();

    static JKRAramBlock *MainRamToAram(const u8 *src, u32 dst, u32 size, s32 r6, u32 r7,
            JKRHeap *heap, s32 r9, u32 *r10);
    static JKRAramBlock *AramToMainRam(u32 src, u8 *dst, u32 size, s32 r6, u32 r7, JKRHeap *heap,
            s32 r9, u32 *r10);

private:
    JKRAram(u32 audioBufSize, u32 graphBufSize, s32 priority);
    ~JKRAram() override;
    void *run() override;

    u32 m_audioBuf;
    u32 m_audioBufSize;
    u32 m_graphBuf;
    u32 m_graphBufSize;
    u32 m_userBuf;
    u32 m_userBufSize;
    JKRAramHeap *m_heap;
    u32 m_stackIndex;
    u8 _9c[0xa4 - 0x9c];

    static JKRAram *s_aramObject;
};
size_assert(JKRAram, 0xa4);
