#pragma once

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class BombStart2D {
public:
    void REPLACED(init)();
    REPLACE void init();

private:
    u8 _00[0x1c - 0x00];
    J2DScreen *m_screen;
    u8 _20[0x24 - 0x20];
};
size_assert(BombStart2D, 0x24);
