#pragma once

#include "jsystem/JUTFader.hh"

class JFWDisplay {
public:
    virtual void beginRender();
    virtual void endRender();
    virtual void endFrame();

    u32 getFaderStatus() const;
    void setFaderColor(u8 r, u8 g, u8 b);
    bool startFadeIn(s32 duration);
    bool startFadeOut(s32 duration);

private:
    JUTFader *m_fader;
    u8 _08[0x4c - 0x08];
};
size_assert(JFWDisplay, 0x4c);
