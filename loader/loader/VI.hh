#pragma once

#include <common/Types.hh>

class VI {
public:
    struct Color {
        u8 y0;
        u8 u;
        u8 y1;
        u8 v;

        static const Color Black;
        static const Color White;
    };

    static void Init();
    static bool IsProgressive();
    static u16 GetXFBWidth();
    static u16 GetXFBHeight();
    static Color ReadFromXFB(u16 x, u16 y);
    static void WriteToXFB(u16 x, u16 y, Color color);
    static void FlushXFB();

private:
    VI();

    static u16 s_xfbWidth;
    static u16 s_xfbHeight;
    static u32 *s_xfb;
};
