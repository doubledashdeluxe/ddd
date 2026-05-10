#pragma once

#include "game/KartGamePad.hh"

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class Pause2D {
public:
    class State {
    public:
        enum {
            Reset = 0,
            SlideIn = 1,
            Cooldown = 2,
            Idle = 3,
            SlideOut = 4,
        };

    private:
        State();
    };

    Pause2D(JKRHeap *heap, JKRArchive *archive);
    void init();
    void REPLACED(draw)();
    REPLACE void draw();
    void calc(const KartGamePad *pad);

    static u32 REPLACED(GetState)();
    REPLACE static u32 GetState();
    static void SetState(u32 state);
    static u32 GetSelector();

private:
    struct HioNode {
        u8 _00[0x48 - 0x00];
        u8 : 7;
        bool isVisible : 1;
        u8 _49[0x4c - 0x49];
    };
    size_assert(HioNode, 0x4c);

    struct Line {
        J2DAnmBase *anmTransform;
        J2DPane *pane;
        u8 _8[0xc - 0x8];
    };
    size_assert(Line, 0xc);

    typedef void (Pause2D::*SelectorFunc)(const KartGamePad *pad);
    typedef void (Pause2D::*SetDrawFunc)();

    void setPadText();
    void selectorTA(const KartGamePad *pad);
    void selectorGP(const KartGamePad *pad);
    void selectorVS(const KartGamePad *pad);
    void selectorMG(const KartGamePad *pad);
    void setDrawTA();
    void setDrawGP();
    void setDrawVS();
    void setDrawMG();

    HioNode *m_hioNode;
    J2DGraphContext *m_graphContext;
    J2DScreen *m_screen;
    J2DAnmBase *m_anmTransform;
    J2DAnmBase *m_anmColors;
    u8 _014[0x02c - 0x014];
    J2DPicture *m_namePictures[6];
    J2DPane *m_foregroundPane;
    J2DPicture *m_starPictures[12];
    TVec2<f32> m_starPositions[12];
    J2DPicture *m_cursorPictures[2];
    u8 _0e0[0x114 - 0x0e0];
    Line m_lines[6];
    SelectorFunc m_selector;
    SetDrawFunc m_setDraw;
};
size_assert(Pause2D, 0x174);
