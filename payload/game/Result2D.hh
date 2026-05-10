#pragma once

#include "game/KartGamePad.hh"

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class Result2D {
public:
    class State {
    public:
        enum {
            Selector = 2,
            OnlineTime = 4,  // Added
            OnlineMatch = 5, // Added
            OnlineTotal = 6, // Added
        };

    private:
        State();
    };

    struct Online {};

    Result2D(JKRHeap *heap);
    Result2D(JKRHeap *heap, Online online);
    void REPLACED(init)();
    REPLACE void init();
    void draw();
    void REPLACED(calc)(const KartGamePad *pad);
    REPLACE void calc(const KartGamePad *pad);
    bool getAnmEnd() const;
    void end();

    static void REPLACED(SetGPClr)();
    REPLACE static void SetGPClr();
    static u32 GetState();
    static u32 GetSelector();

private:
    struct HioNode {
        u8 _00[0x4a - 0x00];
        u8 : 7;
        bool isVisible : 1;
        u8 _4b[0x4c - 0x4b];
    };
    size_assert(HioNode, 0x4c);

    struct Data2D {
        TVec2<f32> pos;
        J2DPicture *picture;
    };
    size_assert(Data2D, 0xc);

    struct Selector {
        Data2D leftLines[3];
        Data2D rightLines[3];
        Data2D name;
        Data2D leftCursor;
        Data2D rightCursor;
    };
    size_assert(Selector, 0x6c);

    struct GPLayout {
        Data2D courseLine;
        Data2D courseName;
        Data2D cupName;
        Data2D cupIcon;
        Data2D levelDigits[3];
        Data2D levelChars[3];
        Data2D mirror;
        f32 lineYs[8];
        f32 transXs[3];
    };
    size_assert(GPLayout, 0xb0);

    struct GPLine {
        struct Character {
            Data2D box;
            Data2D icon;
        };

        Data2D cursors[3];
        Character characters[2];
        Data2D rank;
        Data2D timeDigits[7];
        Data2D timeSeps[2];
        Data2D plusDigits[2];
        Data2D arrow;
    };
    size_assert(GPLine, 0xf0);

    struct GPPoint {
        Data2D points;
        Data2D pointDigits[3];
    };
    size_assert(GPPoint, 0x30);

    struct GPScale {
        Vec2f line;
        Vec2f point;
    };
    size_assert(GPScale, 0x10);

    class GPLineModifier {
    protected:
        void modify(GPLine &gpLine);

    private:
        virtual void modify(Data2D &data) = 0;
    };

    class GPLineScaler : public GPLineModifier {
    public:
        GPLineScaler(GPLine &gpLine, const TVec2<f32> &scale);

    private:
        void modify(Data2D &data) override;

        TVec2<f32> m_scale;
    };

    class GPPointModifier {
    protected:
        void modify(GPPoint &gpPoint);

    private:
        virtual void modify(Data2D &data) = 0;
    };

    class GPPointScaler : public GPPointModifier {
    public:
        GPPointScaler(GPPoint &gpPoint, const TVec2<f32> &scale);

    private:
        void modify(Data2D &data) override;

        TVec2<f32> m_scale;
    };

    typedef void (Result2D::*DrawFunc)();
    typedef void (Result2D::*SetDrawFunc)();
    typedef void (Result2D::*SelectorFunc)(const KartGamePad *pad);

    void drawGP();
    void drawOnline();
    void setDrawGP();
    void setDrawOnline();
    void setSelectorScale(s32 index);
    void selectorGP(const KartGamePad *pad);
    void selectorOnline(const KartGamePad *pad);
    void selectorCommon();
    void calcOnlineTime();
    void calcOnlineMatch();
    void calcOnlineTotal();
    void calcOnlineCommon(bool hasPrev, bool hasNext);
    void calcSelector();
    void setOnlineTime();
    void setOnlineMatch();
    void setOnlineTotal();
    void setOnlineCommon(bool isTotal);

    void getRaceRankLayoutAlpha(s32 frame, u8 &alpha) const;
    void getGPRankLayoutAlpha(s32 frame, u8 &alpha) const;
    void getRaceRankLinePos(s32 frame, s32 rank, f32 &x) const;
    void getGPRankLinePos(s32 frame, s32 rank, f32 &x) const;
    void getRaceRankLineScale(s32 frame, s32 rank, Vec2f &lineScale, Vec2f &pointScale) const;
    void getGPRankLineScale(s32 frame, s32 rank, Vec2f &lineScale, Vec2f &pointScale) const;
    void getKartColor(s32 kartIndex, GXColor &color) const;
    void getCharacterColor(s32 kartIndex, s32 characterIndex, GXColor &boxColor,
            J2DPicture::CornerColors &charColors) const;
    void getSelectorPos(s32 frame, f32 &x) const;
    f32 getSelectorScale(s32 frame) const;

    static void Setup(Data2D &data, J2DScreen *screen, const char *format, ...);

    HioNode *m_hioNode;
    J2DGraphContext *m_graphContext;
    u8 _0008[0x004c - 0x0008];
    J2DScreen *m_overScreen;
    J2DScreen *m_selectorScreens[6];
    u32 m_selectorCount;
    u8 _006c[0x0078 - 0x006c];
    Selector m_selectors[6];
    J2DScreen *m_gpLayoutScreen;
    J2DScreen *m_gpRaceScreen;
    J2DScreen *m_gpLineScreens[8];
    J2DScreen *m_gpPointScreens[8];
    GPLayout m_gpLayout;
    u8 _03f8[0x04c4 - 0x03f8];
    GPLine m_gpLines[8];
    TVec2<f32> m_gpPointPositions[2];
    GPPoint m_gpPoints[8];
    f32 m_selectorY;
    u8 _0dd8[0x197c - 0x0dd8];
    s32 m_frame;
    s32 m_endFrame;
    s32 m_overBackAlpha;
    f32 m_selectorX;
    s32 m_anmEndFrame;
    s32 m_lineColorFrame;
    u8 _1994[0x199c - 0x1994];
    s32 m_pointDiffs[8];
    f32 m_gpLineXs[8];
    GPScale m_gpScales[8];
    u8 m_gpLayoutAlpha;
    bool m_hasOnlineTime;  // Added (was padding)
    bool m_hasOnlineTotal; // Added (was padding)
    u8 _1a5f[0x1af4 - 0x1a5f];
    s32 m_selectorScaleFrame;
    f32 m_selectorScale;
    s32 m_selectorCursorFrame;
    f32 m_selectorCursorPrevY;
    f32 m_selectorCursorCurrY;
    f32 m_selectorCursorNextY;
    u8 _1b0c[0x1b14 - 0x1b0c];
    TVec2<f32> m_gpLineItemScales[10];
    TVec2<f32> m_gpPointItemScales[10];
    u8 _1bb4[0x1bf4 - 0x1bb4];
    DrawFunc m_draw;
    SetDrawFunc m_setDraw;
    SelectorFunc m_selector;
    u8 _1c18[0x1c24 - 0x1c18];
    J2DPicture *m_playerNamePictures[8][2][3];
    J2DPicture *m_plusDigitPictures[8][4];

    static bool s_isVisible;
    static u32 s_state;
    static u32 s_selector;
};
size_assert(Result2D, 0x1c24 + sizeof(J2DPicture *[8][2][3]) + sizeof(J2DPicture *[8][4]));
