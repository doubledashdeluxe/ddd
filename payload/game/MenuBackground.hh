#pragma once

#include <jsystem/J2DScreen.hh>

class MenuBackground {
public:
    void draw(const J2DGraphContext *graphContext);
    void calc();

    static MenuBackground *Create(JKRArchive *archive);
    static void Destroy();
    static MenuBackground *Instance();

private:
    MenuBackground(JKRArchive *archive);
    ~MenuBackground();

    J2DScreen m_screen;
    u8 _118[0x120 - 0x118];

    static MenuBackground *s_instance;
};
size_assert(MenuBackground, 0x120);
