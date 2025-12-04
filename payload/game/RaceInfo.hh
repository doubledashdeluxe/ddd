#pragma once

#include "game/KartGamePad.hh"
#include "game/KartInfo.hh"

class RaceInfo {
public:
    u32 getRaceMode() const;
    u32 getRaceLevel() const;
    s16 getKartCount() const;
    s16 getConsoleCount() const;
    s16 getStatusCount() const;
    const KartInfo &getKartInfo(u32 index) const;
    s16 getAwardKartNo() const;

    bool isRace() const;
    bool isBattle() const;

    void reset();
    void setConsoleTarget(u32 index, u32 targetKart, bool isDemoKart);
    void settingForWaitDemo(bool isLast);
    void setRace(u32 raceMode, u32 kartCount, u32 playerCount, u32 consoleCount, u32 statusCount);
    void setKart(u32 index, u32 kartID, u32 frontChararacterID, KartGamePad *frontPad,
            u32 backCharacterID, KartGamePad *backPad);
    void setRaceLevel(u32 raceLevel);

    static RaceInfo &Instance();

private:
    u8 _000[0x008 - 0x000];

public:
    u32 m_raceMode;

private:
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
    s16 m_statusCount;
    u8 _024[0x030 - 0x024];
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
