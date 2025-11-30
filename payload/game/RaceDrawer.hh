#pragma once

#include "game/KartDrawer.hh"
#include "game/ZCaptureMgr.hh"

class RaceDrawer {
public:
    KartDrawer *kartDrawer(u32 index);
    ZCaptureMgr *zCaptureMgr();

private:
    u8 _00[0x44 - 0x00];
    KartDrawer *m_kartDrawers;
    u8 _48[0x54 - 0x48];
    ZCaptureMgr *m_zCaptureMgr;
    u8 _58[0x64 - 0x58];
};
size_assert(RaceDrawer, 0x64);
