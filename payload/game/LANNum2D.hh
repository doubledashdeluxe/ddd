#pragma once

#include <jsystem/J2DScreen.hh>

class LANNum2D {
public:
    void REPLACED(init)();
    REPLACE void init();
    void REPLACED(start)();
    REPLACE void start();
    void REPLACED(start2)();
    REPLACE void start2();

private:
    void setText(const char *prefix, const char *text);

    u8 _000[0x074 - 0x000];
    J2DScreen *m_screen;
    u8 _078[0x10c - 0x078];
};
size_assert(LANNum2D, 0x10c);
