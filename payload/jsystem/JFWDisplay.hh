#pragma once

#include "jsystem/JUTFader.hh"
#include "jsystem/JUTXfb.hh"

class JFWDisplay {
public:
    void delayFrame();
    u32 delayedFrames() const;

    REPLACE void beginRender();
    void endDraw();
    void endRender();
    void endFrame();
    bool drawing() const;

    u32 getFaderStatus() const;
    void setFaderColor(u8 r, u8 g, u8 b);
    bool startFadeIn(s32 duration);
    bool startFadeOut(s32 duration);
    bool ensureIn(s32 duration);
    bool ensureOut(s32 duration);

    REPLACE static JFWDisplay *Create(GXRenderModeObj *renderMode, JKRHeap *heap, s32 xfbCount,
            bool hasAlpha);
    static JFWDisplay *Instance();

private:
    JFWDisplay(JKRHeap *heap, s32 xfbCount, bool hasAlpha);

    void clearEfbInit();
    void clearEfb();

    static void WaitForTick(u32 tickRate, u16 frameRate);

    u32 m_delayedFrames; // Modified
    JUTFader *m_fader;
    GXColor m_clearColor;
    u32 m_clearZ;
    JUTXfb *m_xfb;
    u16 m_gamma;
    u32 m_drawDoneMethod;
    u16 m_frameRate;
    u32 m_tickRate;
    bool m_hasAlpha;
    u16 m_clamp;
    f32 m_combinationRatio;
    u32 m_frameStart;
    u32 m_frameDuration;
    u32 m_relFrameStart;
    u32 m_frameCount;
    u32 m_frameIndex;
    bool m_drawing;
    void *m_callback;
    s16 m_drawingXfbIndex;

    static JFWDisplay *s_instance;
};
size_assert(JFWDisplay, 0x4c);
