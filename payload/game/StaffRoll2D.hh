#pragma once

#include <portable/Types.hh>

class StaffRoll2D {
public:
    void calc();

private:
    u8 _0000[0x4598 - 0x0000];
};
size_assert(StaffRoll2D, 0x4598);
