#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class SequenceInfo {
public:
    u32 getBattleMode() const;

    void REPLACED(init)();
    REPLACE void init();

    static SequenceInfo &Instance();

private:
    u8 _000[0x028 - 0x000];

public:
    bool m_fromPause;
    u8 m_packIndex;            // Added (was padding)
    u8 m_padCount : 3;         // Added (was padding)
    bool m_hasOnlineNames : 1; // Added (was padding)
    u8 m_statusCount : 2;      // Added (was padding)
    u8 : 2;

private:
    u8 _02b[0x038 - 0x02b];
    u32 m_battleMode;

public:
    u32 m_mapIndex;

private:
    u8 _040[0x44c - 0x040];

    static SequenceInfo s_instance;
};
size_assert(SequenceInfo, 0x44c);
