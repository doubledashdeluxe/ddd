#pragma once

#include "game/MinimapConfig.hh"

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <jsystem/TVec2.hh>
#include <jsystem/TVec3.hh>
#include <payload/Replace.hh>

class Race2D {
public:
    void REPLACED(init)();
    REPLACE void init();
    void setMinimapConfig(const MinimapConfig &minimapConfig);

    static Race2D *Instance();

private:
    void REPLACED(getMapPos)(s32 r4, const Vec3f &pos, Vec2f &mapPos);
    REPLACE void getMapPos(s32 r4, const Vec3f &pos, Vec2f &mapPos);
    void REPLACED(getCharacterInfo)(s32 r4, s32 r5, f32 &x, f32 &y, f32 &scale);
    REPLACE void getCharacterInfo(s32 r4, s32 r5, f32 &x, f32 &y, f32 &scale);
    void REPLACED(getItemInfo)(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale);
    REPLACE void getItemInfo(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale);
    void REPLACED(getStartCharPos)(s32 r4, s32 r5, f32 &f1);
    REPLACE void getStartCharPos(s32 r4, s32 r5, f32 &f1);

    u8 _0000[0x0230 - 0x0000];
    J2DScreen *m_raceScreen;
    J2DScreen *m_maskScreen;
    J2DPicture *m_playerNumberPictures[4][8];
    u8 _02b8[0x0338 - 0x02b8];
    J2DScreen *m_shineIndicationScreens[4];
    u8 _0348[0x04b4 - 0x0348];
    J2DPicture *m_specialItemPictures[4][2];
    u8 _04d4[0x1110 - 0x04d4];
    MinimapConfig m_minimapConfig;
    u8 _1124[0x4ec4 - 0x1124];

    static Race2D *s_instance;
};
size_assert(Race2D, 0x4ec4);
