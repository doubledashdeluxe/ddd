#pragma once

extern "C" {
#include <dolphin/GX.h>
}
#include <payload/Replace.h>

class JUTVideo {
public:
    void REPLACED(setRenderMode)(const GXRenderModeObj *renderMode);
    REPLACE void setRenderMode(const GXRenderModeObj *renderMode);

    static void REPLACED(DummyNoDrawWait)();
    REPLACE static void DummyNoDrawWait();

private:
    JUTVideo(const GXRenderModeObj *renderMode);

    u8 _00[0x58 - 0x00];

    static GXRenderModeObj s_renderMode;
};
