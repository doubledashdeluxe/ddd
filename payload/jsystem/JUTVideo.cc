#include "JUTVideo.hh"

#include "jsystem/JFWDisplay.hh"
#include "jsystem/JUTXfb.hh"

extern "C" {
#include <dolphin/VI.h>
}

void JUTVideo::setRenderMode(const GXRenderModeObj * /* renderMode */) {
    if (VIGetTvFormat() == VI_NTSC && VIGetDTVStatus()) {
        s_renderMode.viTVmode = 0x2;
    } else {
        s_renderMode.viTVmode = VIGetTvFormat() << 2;
    }
    s_renderMode.fbWidth = 608;
    s_renderMode.efbHeight = 448;
    if (VIGetTvFormat() == VI_PAL) {
        s_renderMode.xfbHeight = 538;
        s_renderMode.viXOrigin = 25;
        s_renderMode.viYOrigin = 18;
        s_renderMode.viWidth = 670;
        s_renderMode.viHeight = 538;
    } else {
        s_renderMode.xfbHeight = 448;
        s_renderMode.viXOrigin = 27;
        s_renderMode.viYOrigin = 16;
        s_renderMode.viWidth = 666;
        s_renderMode.viHeight = 448;
    }
    s_renderMode.field_rendering = 0;
    s_renderMode.aa = 0;
    if (VIGetTvFormat() == VI_NTSC && VIGetDTVStatus()) {
        s_renderMode.xfbMode = VI_XFBMODE_SF;
    } else {
        s_renderMode.xfbMode = VI_XFBMODE_DF;
    }
    for (size_t i = 0; i < 12; i++) {
        for (size_t j = 0; j < 2; j++) {
            s_renderMode.sample_pattern[i][j] = 6;
        }
    }
    s_renderMode.vfilter[0] = 0;
    s_renderMode.vfilter[1] = 0;
    s_renderMode.vfilter[2] = 21;
    s_renderMode.vfilter[3] = 22;
    s_renderMode.vfilter[4] = 21;
    s_renderMode.vfilter[5] = 0;
    s_renderMode.vfilter[6] = 0;

    REPLACED(setRenderMode)(&s_renderMode);
}

u32 JUTVideo::FrameStart() {
    return s_frameStart;
}

u32 JUTVideo::FrameDuration() {
    return s_frameDuration;
}

void JUTVideo::PreRetraceProc(u32 retraceCount) {
    JUTXfb *xfb = JUTXfb::Instance();
    JFWDisplay *display = JFWDisplay::Instance();
    if (xfb && xfb->m_state == JUTXfb::State::Drawing && display) {
        display->delayFrame();
    }

    REPLACED(PreRetraceProc)(retraceCount);
}

GXRenderModeObj JUTVideo::s_renderMode;
