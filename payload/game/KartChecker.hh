#pragma once

#include <portable/Types.hh>

class KartChecker {
public:
    bool lapRenewal() const;
    bool raceEnd() const;
    s32 lap() const;
    s32 rank() const;

private:
    u8 _00[0x28 - 0x00];
    bool m_lapRenewal;
    bool m_raceEnd;
    u8 _2a[0x2c - 0x2a];
    s32 m_lap;
    u8 _30[0x88 - 0x30];
    s32 m_rank;
    u8 _8c[0xc0 - 0x8c];
};
size_assert(KartChecker, 0xc0);
