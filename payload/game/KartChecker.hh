#pragma once

#include <portable/Types.hh>

class KartChecker {
public:
    bool lapRenewal() const;
    bool raceEnd() const;
    s32 lap() const;

private:
    u8 _00[0x28 - 0x00];
    bool m_lapRenewal;
    bool m_raceEnd;
    u8 _2a[0x2c - 0x2a];
    s32 m_lap;
    u8 _30[0xc0 - 0x30];
};
size_assert(KartChecker, 0xc0);
