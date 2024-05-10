#pragma once

#include <common/Types.hh>

class SequenceInfo {
public:
    u32 getBattleMode() const;

    void init();

    static SequenceInfo &Instance();

private:
    u8 _000[0x028 - 0x000];

public:
    bool m_fromPause;
    u8 m_packIndex; // Added

private:
    u8 _02a[0x038 - 0x02a];
    u32 m_battleMode;

public:
    u32 m_mapIndex;

private:
    u8 _040[0x44c - 0x040];

    static SequenceInfo s_instance;
};
size_assert(SequenceInfo, 0x44c);
