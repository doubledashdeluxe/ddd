#pragma once

#include <jsystem/J2DScreen.hh>
#include <jsystem/JKRHeap.hh>

class CanNotSaveG2D {
public:
    CanNotSaveG2D(JKRHeap *heap);
    void setupOnline();
    void draw();
    void calc();

private:
    J2DAnmBase *m_anmTransform;
    u8 _04[0x1c - 0x04];
    J2DScreen *m_screen;
    bool m_isActive;
    u8 _21[0x24 - 0x21];
};
size_assert(CanNotSaveG2D, 0x24);
