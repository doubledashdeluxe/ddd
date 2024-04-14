#pragma once

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class Result2D {
public:
    void REPLACED(init)();
    REPLACE void init();

private:
    u8 _0000[0x004c - 0x0000];
    J2DScreen *m_overScreen;
    u8 _0050[0x1c24 - 0x0050];
};
size_assert(Result2D, 0x1c24);
