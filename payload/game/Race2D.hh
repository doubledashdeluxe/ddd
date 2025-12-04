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
    size_assert(Scissor, 0x10);

    struct Console {
        u8 _000[0x048 - 0x000];
        Vec2f timePositions[10];
        u8 _098[0x4d0 - 0x098];
        Vec2f lapTimePositions[9][11];
        u8 _7e8[0x854 - 0x7e8];
        Vec2f rankPositions[8];
        u8 _894[0x8e8 - 0x894];
    };
    size_assert(Console, 0x8e8);

    struct CharacterIndicationAnm {
        s32 frame;
        s32 characterIndices[2];
    };
    size_assert(CharacterIndicationAnm, 0xc);

    struct CharacterIndication {
        struct Character {
            K2DPicture *windowPicture;
            K2DPicture *iconPicture;
        };

        u8 _00[0x04 - 0x00];
        Character characters[2];
        K2DPicture *rankPicture;
        u32 rank;
        u8 _1c[0x2c - 0x1c];
        Vec2f pos;
        u8 _34[0x36 - 0x34];
        u8 alpha;
        u8 _37[0x38 - 0x37];
    };
    size_assert(CharacterIndication, 0x38);

    struct KartIndexComparator {
        bool operator()(const u32 &a, const u32 &b);

        Race2D &race2D;
    };

    struct PlayerMarkIndexComparator {
        bool operator()(const u32 &a, const u32 &b);

        Array<s32, 8> depths;
    };

    void setup();
    void REPLACED(drawPlayerMark)();
    REPLACE void drawPlayerMark();
    void REPLACED(draw)();
    REPLACE void draw();
    void REPLACED(drawCourse)();
    REPLACE void drawCourse();
    void REPLACED(calcPlayerMark)();
    REPLACE void calcPlayerMark();
    void REPLACED(getMapPos)(s32 r4, const Vec3f &pos, Vec2f &mapPos);
    REPLACE void getMapPos(s32 r4, const Vec3f &pos, Vec2f &mapPos);
    void REPLACED(getItemInfo)(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale);
    REPLACE void getItemInfo(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale);
    void getCharacterInfo(s32 characterIndex, s32 frame, f32 &x, f32 &y, f32 &scale);
    void REPLACED(getCharacterColor)(s32 kartIndex, s32 characterIndex, s32 frame,
            GXColor &windowColor, GXColor &iconColor, u8 &iconAlpha);
    REPLACE void getCharacterColor(s32 kartIndex, s32 characterIndex, s32 frame,
            GXColor &windowColor, GXColor &iconColor, u8 &iconAlpha);
    void getStartScaleA(s32 index, f32 &scale);
    void getStartScaleB(s32 index, f32 &scale);
    void REPLACED(getStartCharPos)(s32 frame, s32 index, f32 &x);
    REPLACE void getStartCharPos(s32 frame, s32 index, f32 &x);
    void getGoalCharPos(s32 frame, s32 index, f32 &x);
    f32 getThunderCharScale(s32 frame);
    void getStartLapTimePos(s32 frame, s32 index, f32 &x);
    void getGoalLapTimePos(s32 frame, s32 index, f32 &x);

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
    u8 _0594[0x0694 - 0x0594];
    K2DPicture *m_timePictures[2][10];
    K2DPicture *m_lapTimePictures[2][9][11];
    u8 _09fc[0x1000 - 0x09fc];
    Vec2f m_playerMarkPositions[4][8];
    u8 _1100[0x1110 - 0x1100];
    MinimapConfig m_minimapConfig;
    u8 _1124[0x1588 - 0x1124];
    Console m_consoles[4];
    u8 _3928[0x4c3c - 0x3928];
    CharacterIndicationAnm m_characterIndicationAnms[8];
    CharacterIndication m_characterIndications[8];
    u8 _4e5c[0x4e9c - 0x4e5c];
    s32 m_characterIndicationColorFrame;
    u8 _4ea0[0x4ea4 - 0x4ea0];
    Vec2f m_characterIndicationIconPos;
    Vec2f m_characterIndicationRankPos;
    s32 m_frame;
    u8 _4eb8[0x4ec0 - 0x4eb8];
    bool m_isVisible;
    u8 _4ec1[0x4ec4 - 0x4ec1];

    static Race2D *s_instance;
    static GXColor s_playerNumberColors[16];
    static f32 s_spinRotate[8];
    static s32 s_thunderAnm[8];
};
size_assert(Race2D, 0x4ec4);
