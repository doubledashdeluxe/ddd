#pragma once

#include <portable/Types.hh>

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

    bool isProgressive() const;
    u16 getXFBWidth() const;
    u16 getXFBHeight() const;
    u32 getXFBSize() const;
    u32 *getXFB() const;
    Color readFromXFB(u16 x, u16 y);
    void writeToXFB(u16 x, u16 y, Color color);

    static void Init();
    static void Init(VI *instance);
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
