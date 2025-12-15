#pragma once

#include <portable/Types.hh>

class KartChecker {
public:
    bool lapRenewal() const;
    bool raceEnd() const;

private:
    u8 _00[0x28 - 0x00];
    bool m_lapRenewal;
    bool m_raceEnd;
    u8 _2a[0xc0 - 0x2a];
};
size_assert(KartChecker, 0xc0);
