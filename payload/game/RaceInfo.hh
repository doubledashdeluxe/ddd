#pragma once

#include <common/Types.hh>

class RaceInfo {
public:
    u32 getRaceLevel() const;

    static RaceInfo &Instance();

private:
    u8 _000[0x010 - 0x000];
    u32 m_raceLevel;
    u8 _014[0x2a0 - 0x014];

    static RaceInfo s_instance;
};
size_assert(RaceInfo, 0x2a0);
