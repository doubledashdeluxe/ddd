#pragma once

#include <portable/Types.hh>

class J2DManager {
public:
    static u32 KartStatus(u32 kartIndex);

private:
    u8 _00[0x48 - 0x00];
};
size_assert(J2DManager, 0x48);
