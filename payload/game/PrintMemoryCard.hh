#pragma once

#include <jsystem/JKRHeap.hh>

class PrintMemoryCard {
public:
    PrintMemoryCard(JKRHeap *heap);

    void reset();
    void init(u32 messageID);
    void draw();
    void calc();
    s32 getFinalChoice() const;
    void ack();
    void ack(u32 soundID);

private:
    u8 _00[0x0c - 0x00];
    bool _c;
    bool _d;
    bool _e;
    u8 _0f[0x14 - 0x0f];
    s32 m_finalChoice;
    s32 m_currentChoice;
    s32 m_state;
    s32 m_frame;
    u8 _24[0x30 - 0x24];
};
size_assert(PrintMemoryCard, 0x30);
