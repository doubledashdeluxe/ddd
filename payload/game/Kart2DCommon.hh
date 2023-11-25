#pragma once

#include <jsystem/J2DPicture.hh>

class Kart2DCommon {
public:
    void changeNumberTexture(s32 number, J2DPicture **pictures, u8 pictureCount, bool padWithZeros,
            bool fillWithHyphens);
    ResTIMG *getBattleIcon(u32 index);

    static Kart2DCommon *Instance();

private:
    u8 _000[0x198 - 0x000];
    ResTIMG *m_battleIcons[3];
    u8 _1a4[0x1bc - 0x1a4];
};
size_assert(Kart2DCommon, 0x1bc);
