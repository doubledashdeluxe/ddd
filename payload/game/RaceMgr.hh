#pragma once

#include "game/Award2D.hh"
#include "game/KartChecker.hh"
#include "game/KartLoader.hh"
#include "game/RaceDirector.hh"
#include "game/RaceDrawer.hh"
#include "game/StaffRoll2D.hh"

#include <payload/Replace.hh>

class RaceMgr {
public:
    RaceDirector *raceDirector() const;
    RaceDrawer *raceDrawer() const;
    u8 loopFrame() const;
    u32 frame() const;
    bool isReplay() const;
    KartChecker *kartChecker(u32 index) const;
    KartLoader *kartLoader(u32 index) const;
    void calcRace();
    void calcRace(s32 adjustment);
    void updateRace();

    static RaceMgr *Instance();

private:
    void framework();
    void checkRank();

    u8 _00[0x18 - 0x00];
    RaceDirector *m_raceDirector;
    RaceDrawer *m_raceDrawer;
    u8 _20[0x22 - 0x20];
    u8 m_loopFrame;
    u32 m_frame;
    u8 _28[0x2c - 0x28];
    u16 : 15;
    bool m_isReplay : 1;
    u8 _2e[0x48 - 0x2e];
    KartChecker *m_kartCheckers[8];
    KartLoader *m_kartLoaders[8];
    Award2D *m_award2D;
    StaffRoll2D *m_staffRoll2D;
    u8 _90[0xb8 - 0x90];

    static RaceMgr *s_instance;
};
size_assert(RaceMgr, 0xb8);
