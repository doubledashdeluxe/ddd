#pragma once

#include "jsystem/JUTFader.hh"

class JFWDisplay {
public:
    u32 getFaderStatus() const;
    void setFaderColor(u8 r, u8 g, u8 b);
    bool startFadeIn(s32 duration);
    bool startFadeOut(s32 duration);

private:
    u8 _00[0x04 - 0x00];
    JUTFader *m_fader;
    u8 _08[0x4c - 0x08];
};
size_assert(JFWDisplay, 0x4c);
