#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class KartCheck {
public:
    bool REPLACED(checkBodyHit)(u32 i0, u32 i1) const;
    REPLACE bool checkBodyHit(u32 i0, u32 i1) const;

private:
    u8 _0[0x8 - 0x0];
};
size_assert(KartCheck, 0x8);
