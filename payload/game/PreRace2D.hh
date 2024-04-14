#pragma once

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class PreRace2D {
public:
    void REPLACED(init)();
    REPLACE void init();

private:
    u8 _00[0x20 - 0x00];
    J2DScreen *m_screen;
    u8 _24[0x88 - 0x24];
    J2DPicture *m_modePictures[22];
};
size_assert(PreRace2D, 0xe0);
