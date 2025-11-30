#pragma once

#include "game/K2DPicture.hh"
#include "game/MinimapConfig.hh"

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <jsystem/TVec2.hh>
#include <jsystem/TVec3.hh>
#include <payload/Replace.hh>
#include <portable/Array.hh>

class Race2D {
public:
    void REPLACED(init)();
    REPLACE void init();
    void setMinimapConfig(const MinimapConfig &minimapConfig);

    static Race2D *Instance();
    static GXColor GetPlayerNumberColor(u32 index);
    static J2DPicture::CornerColors GetCornerColors(u32 index);

private:
    struct Scissor {
        u32 x;
        u32 y;
        u32 w;
        u32 h;
    };

    struct PlayerMarkIndexComparator {
        bool operator()(const u32 &a, const u32 &b);

        Array<s32, 8> depths;
    };

    void setup();
    void REPLACED(drawPlayerMark)();
    REPLACE void drawPlayerMark();
    void REPLACED(calcPlayerMark)();
    REPLACE void calcPlayerMark();
    void REPLACED(getMapPos)(s32 r4, const Vec3f &pos, Vec2f &mapPos);
    REPLACE void getMapPos(s32 r4, const Vec3f &pos, Vec2f &mapPos);
    void REPLACED(getCharacterInfo)(s32 r4, s32 r5, f32 &x, f32 &y, f32 &scale);
    REPLACE void getCharacterInfo(s32 r4, s32 r5, f32 &x, f32 &y, f32 &scale);
    void REPLACED(getItemInfo)(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale);
    REPLACE void getItemInfo(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale);
    void REPLACED(getStartCharPos)(s32 r4, s32 r5, f32 &f1);
    REPLACE void getStartCharPos(s32 r4, s32 r5, f32 &f1);

    u8 _0000[0x01b4 - 0x0000];
    J2DGraphContext *m_graphContext;
    u8 _01b8[0x01c4 - 0x01b8];
    Scissor m_scissors[4];
    u8 _0204[0x0230 - 0x0204];
    J2DScreen *m_raceScreen;
    J2DScreen *m_maskScreen;
    K2DPicture *m_playerMarkPictures[4][8];
    u32 m_playerMarkIndices[4][8];
    J2DScreen *m_shineIndicationScreens[4];
    u8 _0348[0x04b4 - 0x0348];
    J2DPicture *m_specialItemPictures[4][2];
    K2DPicture *m_playerNamePictures[8][2][3]; // Modified
    u8 _0594[0x1000 - 0x0594];
    Vec2f m_playerMarkPositions[4][8];
    u8 _1100[0x1110 - 0x1100];
    MinimapConfig m_minimapConfig;
    u8 _1124[0x4ec0 - 0x1124];
    bool m_isVisible;
    u8 _4ec1[0x4ec4 - 0x4ec1];

    static Race2D *s_instance;
    static GXColor s_playerNumberColors[16];
};
size_assert(Race2D, 0x4ec4);
