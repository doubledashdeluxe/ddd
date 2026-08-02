#pragma once

#include "game/RaceTime.hh"

#include <payload/Replace.hh>

class KartChecker {
public:
    u32 lapCount() const;
    RaceTime &lapTime(u32 lap) const;
    RaceTime &lapTotalTime(u32 lap) const;
    bool lapRenewal() const;
    bool raceEnd() const;
    s32 lap() const;
    const RaceTime &totalTime() const;
    RaceTime &totalTime();
    s32 rank() const;
    void REPLACED(checkLap)(bool raceEnd);
    REPLACE void checkLap(bool raceEnd);

private:
    void forceRaceEnd();

    u16 : 15;
    bool m_checkLap : 1;
    u8 _02[0x0c - 0x02];
    u32 m_lapCount;
    u8 _10[0x14 - 0x10];
    RaceTime *m_lapTimes;
    RaceTime *m_lapTotalTimes;
    u8 _1c[0x28 - 0x1c];
    bool m_lapRenewal;
    bool m_raceEnd;
    u8 _2a[0x2c - 0x2a];
    s32 m_lap;
    u8 _30[0x84 - 0x30];
    RaceTime m_totalTime;
    s32 m_rank;
    u8 _8c[0xc0 - 0x8c];
};
size_assert(KartChecker, 0xc0);
