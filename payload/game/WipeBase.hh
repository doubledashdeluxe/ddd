#pragma once

extern "C" {
#include <dolphin/GX.h>
}

class WipeBase {
public:
    virtual ~WipeBase();
    virtual void vf_0c();
    virtual void draw(s32 consoleIndex, f32 t, GXColor color);
    virtual void calc(s32 consoleIndex, f32 t, GXColor color);
    virtual void vf_18();
    virtual void vf_1c() = 0;
    virtual void vf_20();
    virtual void vf_24();
    virtual void vf_28();
    virtual void vf_2c() = 0;

private:
    u8 _04[0x1c - 0x04];
};
size_assert(WipeBase, 0x1c);
