#pragma once

#include "common/Types.hh"

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

    bool isProgressive();
    u16 getXFBWidth();
    u16 getXFBHeight();
    Color readFromXFB(u16 x, u16 y);
    void writeToXFB(u16 x, u16 y, Color color);
    void flushXFB();

    static void Init();
    static VI *Instance();

private:
    VI();

    bool m_isProgressive;
    u16 m_xfbWidth;
    u16 m_xfbHeight;
    u32 m_xfbSize;
    u32 *m_xfb;

    static VI *s_instance;
};
