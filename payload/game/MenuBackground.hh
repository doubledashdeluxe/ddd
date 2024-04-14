#pragma once

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class MenuBackground {
public:
    void draw(const J2DGraphContext *graphContext);
    void REPLACED(calc)();
    REPLACE void calc();

    static MenuBackground *Create(JKRArchive *archive);
    static void Destroy();
    static MenuBackground *Instance();

private:
    MenuBackground(JKRArchive *archive);
    ~MenuBackground();

    J2DScreen m_screen;
    J2DAnmBase *m_anmTransform;
    s16 m_anmTransformFrame;
    u8 _11e[0x120 - 0x11e];

    static MenuBackground *s_instance;
};
size_assert(MenuBackground, 0x120);
