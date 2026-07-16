#include "JFWDisplay.hh"

#include "jsystem/J2DOrthoGraph.hh"
#include "jsystem/JUTProcBar.hh"
#include "jsystem/JUTVideo.hh"

#include <cube/VI.hh>
extern "C" {
#include <dolphin/GXAttr.h>
#include <dolphin/GXPixel.h>
#include <dolphin/GXTexture.h>
#include <dolphin/OSTime.h>
}
#include <payload/PerfOverlay.hh>

void JFWDisplay::delayFrame() {
    m_delayedFrames++;
}

u32 JFWDisplay::delayedFrames() const {
    return m_delayedFrames;
}

void JFWDisplay::beginRender() {
    WaitForTick(m_tickRate, m_frameRate);

    u32 frameStart = OSGetTick();
    m_frameDuration = frameStart - m_frameStart;
    m_frameStart = frameStart;
    m_relFrameStart = frameStart - JUTVideo::FrameStart();

    m_drawing = ++m_frameIndex >= m_frameCount && m_xfb->m_state != JUTXfb::State::Drawn;

    if (m_drawing) {
        m_frameIndex = 0;
        m_xfb->m_drawingXfbIndex = 0;
        m_xfb->m_state = JUTXfb::State::Drawing;

        clearEfb();

        GXInvalidateTexAll();
        GXInvalidateVtxCache();
        GXSetPixelFmt(GX_PF_RGBA6_Z24, GX_ZC_LINEAR);
        GXSetDither(true);
    }
}

void JFWDisplay::endDraw() {
    VI *vi = VI::Instance();
    J2DOrthoGraph orthoGraph(0.0f, 0.0f, vi->getXFBWidth(), vi->getXFBHeight(), -1.0f, 1.0f);
    orthoGraph.setViewport();
    m_fader->REPLACED(control)();
}

bool JFWDisplay::drawing() const {
    return m_drawing;
}

u32 JFWDisplay::getFaderStatus() const {
    if (!m_fader) {
        return JUTFader::Status::In;
    }

    return m_fader->getStatus();
}

void JFWDisplay::setFaderColor(u8 r, u8 g, u8 b) {
    m_fader->setColor(r, g, b);
}

bool JFWDisplay::startFadeIn(s32 duration) {
    if (!m_fader) {
        return true;
    }

    return m_fader->startFadeIn(duration);
}

bool JFWDisplay::startFadeOut(s32 duration) {
    if (!m_fader) {
        return true;
    }

    return m_fader->startFadeOut(duration);
}

bool JFWDisplay::ensureIn(s32 duration) {
    u32 faderStatus = getFaderStatus();
    switch (faderStatus) {
    case JUTFader::Status::Out:
    case JUTFader::Status::FadingOut:
        startFadeIn(duration);
        return false;
    case JUTFader::Status::In:
        return true;
    default:
        return false;
    }
}

bool JFWDisplay::ensureOut(s32 duration) {
    u32 faderStatus = getFaderStatus();
    switch (faderStatus) {
    case JUTFader::Status::Out:
        return true;
    case JUTFader::Status::In:
    case JUTFader::Status::FadingIn:
        startFadeOut(duration);
        return false;
    default:
        return false;
    }
}

JFWDisplay *JFWDisplay::Create(GXRenderModeObj * /* renderMode */, JKRHeap *heap, s32 xfbCount,
        bool hasAlpha) {
    s_instance = new JFWDisplay(heap, xfbCount, hasAlpha);
    return s_instance;
}

JFWDisplay *JFWDisplay::Instance() {
    return s_instance;
}

JFWDisplay::JFWDisplay(JKRHeap *heap, s32 xfbCount, bool hasAlpha)
    : m_fader(nullptr)
    , m_clearColor((GXColor){0, 0, 0, 0})
    , m_clearZ(0xffffff)
    , m_xfb(JUTXfb::Create(heap, xfbCount))
    , m_gamma(0)
    , m_drawDoneMethod(0)
    , m_frameRate(1)
    , m_tickRate(0)
    , m_hasAlpha(hasAlpha)
    , m_clamp(3)
    , m_combinationRatio(0.0f)
    , m_frameStart(OSGetTick())
    , m_frameDuration(0)
    , m_relFrameStart(0)
    , m_frameCount(1)
    , m_frameIndex(0)
    , m_drawing(false)
    , m_callback(nullptr)
    , m_drawingXfbIndex(0) {
    clearEfbInit();
    JUTProcBar::Create()->clear();
}
