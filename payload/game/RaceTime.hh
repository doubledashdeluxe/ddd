#pragma once

#include <portable/Types.hh>

class RaceTime {
public:
    void get(s32 &minutes, s32 &seconds, s32 &milliseconds) const;

private:
    u8 _0[0x4 - 0x0];
};
