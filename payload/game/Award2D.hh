#pragma once

#include <portable/Types.hh>

class Award2D {
public:
    void calc();

private:
    u8 _00[0x58 - 0x00];
};
size_assert(Award2D, 0x58);
