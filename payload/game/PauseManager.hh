#pragma once

#include "game/Pause2D.hh"
#include "game/PrintMemoryCard.hh"
#include "game/Result2D.hh"

#include <payload/Replace.hh>

class PauseManager {
public:
    REPLACE PauseManager(JKRHeap *heap);
    bool paused() const;
    void REPLACED(reset)();
    REPLACE void reset();
    REPLACE void draw();
    void exec();

    static PauseManager *Instance();

private:
    bool m_isLAN;
    bool m_isOnline;    // Added (was padding)
    bool m_paused;      // Added (was padding)
    bool m_wasCanceled; // Added (was padding)
    JKRArchive *m_archive;
    PrintMemoryCard *m_printMemoryCard;
    bool m_hasPrintMemoryCard;
    u8 _0d[0x0e - 0x0d];
    bool m_resultStart;
    u8 _0f[0x11 - 0x0f];
    bool m_isDemo;
    bool m_isVisible;
    bool m_exec;
    s32 m_frame;

public:
    bool m_pauseEnd;

private:
    u8 _19[0x1c - 0x19];
    Pause2D *m_pause2D;
    Result2D *m_result2D;
    J2DGraphContext *m_graphContext;
    s32 m_marioFrame;
    s32 m_wipeOutFrame;
    s32 m_wipeOutDuration;
    s32 m_resultEndFrame;
    u8 _38[0x3c - 0x38];

    static PauseManager *s_instance;
    static s32 s_pauseChoice;
    static s32 s_nextPauseChoice;
};
size_assert(PauseManager, 0x3c);
