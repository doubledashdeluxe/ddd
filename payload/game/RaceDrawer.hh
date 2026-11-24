#pragma once

#include "game/GhostScreen.hh"
#include "game/KartDrawer.hh"
#include "game/ZCaptureMgr.hh"

#include <jsystem/J2DGraphContext.hh>
#include <payload/Replace.hh>

class RaceDrawer {
public:
    void setup();
    void REPLACED(drawMidScene)();
    REPLACE void drawMidScene();
    KartDrawer *kartDrawer(u32 index);
    ZCaptureMgr *zCaptureMgr();

private:
    void drawGhost();
    void drawGhost(u32 console);

    J2DGraphContext *m_graphContext;
    u8 _04[0x44 - 0x04];
    KartDrawer *m_kartDrawers;
    u8 _48[0x4c - 0x48];
    GhostScreen *m_ghostScreens; // Modified
    u8 _50[0x54 - 0x50];
    ZCaptureMgr *m_zCaptureMgr;
    u8 _58[0x64 - 0x58];
};
size_assert(RaceDrawer, 0x64);
