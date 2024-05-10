#pragma once

extern "C" {
#include <dolphin/GX.h>
}

class JUTFader {
public:
    class Status {
    public:
        enum {
            Out = 0,
            In = 1,
            FadingIn = 2,
            FadingOut = 3,
        };

    private:
        Status();
    };

    virtual ~JUTFader();
    virtual bool startFadeIn(s32 duration);
    virtual bool startFadeOut(s32 duration);
    virtual void draw(s32 duration);

    u32 getStatus() const;
    void setColor(u8 r, u8 g, u8 b);

private:
    u32 m_status;
    u8 _08[0x10 - 0x08];
    GXColor m_color;
    u8 _14[0x2c - 0x14];
};
size_assert(JUTFader, 0x2c);
