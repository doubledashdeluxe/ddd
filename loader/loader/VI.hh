#pragma once

#include <common/Types.hh>

class VI {
public:
    static void Init();
    static bool IsProgressive();
    static u16 GetXFBWidth();
    static u16 GetXFBHeight();
    static u8 ReadGrayscaleFromXFB(u16 x, u16 y);
    static void WriteGrayscaleToXFB(u16 x, u16 y, u8 intensity);
    static void FlushXFB();

private:
    VI();

    static u16 s_xfbWidth;
    static u16 s_xfbHeight;
    static u32 *s_xfb;
};
