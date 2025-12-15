#pragma once

#include "game/KartChecker.hh"
#include "game/KartLoader.hh"
#include "game/RaceDrawer.hh"

class RaceMgr {
public:
    RaceDrawer *raceDrawer();
    bool isReplay() const;
    KartChecker *kartChecker(u32 index);
    KartLoader *kartLoader(u32 index);

    static RaceMgr *Instance();

private:
    u8 _00[0x1c - 0x00];
    RaceDrawer *m_raceDrawer;
    u8 _20[0x2c - 0x20];
    u16 : 15;
    bool m_isReplay : 1;
    u8 _2e[0x48 - 0x2e];
    KartChecker *m_kartCheckers[8];
    KartLoader *m_kartLoaders[8];
    u8 _88[0xb8 - 0x88];

    static RaceMgr *s_instance;
};
size_assert(RaceMgr, 0xb8);
