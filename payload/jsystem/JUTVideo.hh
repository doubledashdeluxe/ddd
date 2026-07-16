#pragma once

extern "C" {
#include <dolphin/GX.h>
}
#include <payload/Replace.h>

class JUTVideo {
public:
    void REPLACED(setRenderMode)(const GXRenderModeObj *renderMode);
    REPLACE void setRenderMode(const GXRenderModeObj *renderMode);

    static u32 FrameStart();
    static u32 FrameDuration();

private:
    JUTVideo(const GXRenderModeObj *renderMode);

    static void REPLACED(PreRetraceProc)(u32 retraceCount);
    REPLACE static void PreRetraceProc(u32 retraceCount);

    u8 _00[0x58 - 0x00];

    static u32 s_frameStart;
    static u32 s_frameDuration;
    static GXRenderModeObj s_renderMode;
};
