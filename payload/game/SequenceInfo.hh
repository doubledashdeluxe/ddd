#pragma once

#include "game/KartGamePad.hh"

class SequenceInfo {
public:
    u32 getBattleMode() const;
    const char *modeIconTextureName() const;
    const char *modeNameTextureName() const;
    s32 padPlayer(const KartGamePad *pad) const;

    void init();

    static SequenceInfo &Instance();

private:
    u8 _000[0x028 - 0x000];

public:
    bool m_fromPause;
    u8 m_packIndex;       // Added (was padding)
    bool m_isOnline : 1;  // Added (was padding)
    u8 m_padCount : 3;    // Added (was padding)
    u8 m_statusCount : 3; // Added (was padding)
    u8 : 1;

private:
    u8 _02b[0x038 - 0x02b];
    u32 m_battleMode;

public:
    u32 m_mapIndex;

private:
    u8 _040[0x298 - 0x040];

public:
    u32 m_points[8];
    u32 m_raceRankedKartIndices[8];
    u32 m_gpRankedKartIndices[8];

private:
    u8 _2f8[0x44c - 0x2f8];

    static SequenceInfo s_instance;
};
size_assert(SequenceInfo, 0x44c);
