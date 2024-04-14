#pragma once

#include <common/Types.hh>

class RaceInfo {
public:
    u32 getRaceMode() const;
    u32 getRaceLevel() const;
    s16 getConsoleCount() const;
    s16 getAwardKartNo() const;

    bool isRace() const;
    bool isBattle() const;

    static RaceInfo &Instance();

private:
    u8 _000[0x008 - 0x000];
    u32 m_raceMode;
    u8 _00c[0x010 - 0x00c];
    u32 m_raceLevel;
    u8 _014[0x020 - 0x014];
    s16 m_consoleCount;
    u8 _022[0x120 - 0x022];
    s16 m_awardKartNo;
    u8 _122[0x2a0 - 0x122];

    static RaceInfo s_instance;
};
size_assert(RaceInfo, 0x2a0);
