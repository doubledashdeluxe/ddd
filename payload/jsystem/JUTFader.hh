#pragma once

#include <common/Types.hh>

class JUTFader {
public:
    virtual ~JUTFader();
    virtual bool startFadeIn(s32 duration);
    virtual bool startFadeOut(s32 duration);
    virtual void draw(s32 duration);

private:
    u8 _04[0x2c - 0x04];
};
size_assert(JUTFader, 0x2c);
