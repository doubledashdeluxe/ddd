#pragma once

#include <jsystem/J2DScreen.hh>

class MenuTitleLine {
public:
    class Title {
    public:
        enum {
            SelectMap = 7,
        };

    private:
        Title();
    };

    void drop(u32 title);
    void lift();
    void draw(const J2DGraphContext *graphContext);
    void calc();

    static MenuTitleLine *Create(JKRArchive *archive, JKRHeap *heap);
    static void Destroy();
    static MenuTitleLine *Instance();

private:
    MenuTitleLine(JKRArchive *archive);
    ~MenuTitleLine();

    u8 _000[0x008 - 0x000];
    J2DScreen m_screen;
    u8 _120[0x160 - 0x120];

    static MenuTitleLine *s_instance;
};
size_assert(MenuTitleLine, 0x160);
