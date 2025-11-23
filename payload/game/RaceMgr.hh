#pragma once

#include <portable/Types.hh>

class RaceMgr {
public:
    bool isReplay() const;

    static RaceMgr *Instance();

private:
    u8 _00[0x2c - 0x00];
    u16 : 15;
    bool m_isReplay : 1;
    u8 _2e[0xb8 - 0x2e];

    static RaceMgr *s_instance;
};
size_assert(RaceMgr, 0xb8);
