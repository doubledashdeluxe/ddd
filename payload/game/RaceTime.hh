#pragma once

#include <portable/Types.hh>

class RaceTime {
public:
    void get(s32 &minutes, s32 &seconds, s32 &milliseconds) const;

    u32 m_time;
};
