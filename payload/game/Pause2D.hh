#pragma once

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class Pause2D {
public:
    void REPLACED(init)();
    REPLACE void init();

private:
    u8 _000[0x008 - 0x000];
    J2DScreen *m_screen;
    u8 _00c[0x174 - 0x00c];
};
size_assert(Pause2D, 0x174);
