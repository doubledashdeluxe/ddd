#pragma once

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class Demo2D {
public:
    REPLACE void init();

private:
    u8 _0[0x8 - 0x0];
    J2DScreen *m_screen;
};
size_assert(Demo2D, 0xc);
