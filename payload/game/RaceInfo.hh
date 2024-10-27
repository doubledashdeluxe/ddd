#pragma once

#include "game/KartInfo.hh"

class RaceInfo {
public:
    u32 getRaceMode() const;
    u32 getRaceLevel() const;
    s16 getKartCount() const;
    s16 getConsoleCount() const;
    const KartInfo &getKartInfo(u32 index) const;
    s16 getAwardKartNo() const;

    bool isRace() const;
    bool isBattle() const;

    void reset();
    void settingForWaitDemo(bool isLast);

    static RaceInfo &Instance();

private:
    u8 _000[0x008 - 0x000];
    u32 m_raceMode;
    u8 _00c[0x010 - 0x00c];
    u32 m_raceLevel;
    u8 _014[0x018 - 0x014];

public:
    u16 m_vsLapNum;

private:
    u8 _01a[0x01c - 0x01a];
    s16 m_kartCount;
    u8 _01e[0x020 - 0x01e];
    s16 m_consoleCount;
    u8 _022[0x030 - 0x022];
    KartInfo m_karts[8];
    u8 _0f0[0x120 - 0x0f0];
    s16 m_awardKartNo;
    u8 _122[0x298 - 0x122];

public:
    s32 m_waitDemoResult;

private:
    u8 _29c[0x2a0 - 0x29c];

    static RaceInfo s_instance;
};
size_assert(RaceInfo, 0x2a0);
