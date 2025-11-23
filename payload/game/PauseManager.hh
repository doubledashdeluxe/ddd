#pragma once

#include "game/Pause2D.hh"
#include "game/PrintMemoryCard.hh"
#include "game/Result2D.hh"

#include <payload/Replace.hh>

class PauseManager {
public:
    REPLACE void draw();

private:
    bool m_isLAN;
    u8 _01[0x08 - 0x01];
    PrintMemoryCard *m_printMemoryCard;
    bool m_hasPrintMemoryCard;
    u8 _0d[0x0e - 0x0d];
    bool m_resultStart;
    u8 _0f[0x11 - 0x0f];
    bool m_isDemo;
    bool m_isVisible;
    u8 _13[0x1c - 0x13];
    Pause2D *m_pause2D;
    Result2D *m_result2D;
    u8 _24[0x28 - 0x24];
    s32 m_marioFrame;
    s32 m_wipeOutFrame;
    s32 m_wipeOutDuration;
    u8 _34[0x3c - 0x34];

    static s32 s_nextPauseChoice;
};
size_assert(PauseManager, 0x3c);
