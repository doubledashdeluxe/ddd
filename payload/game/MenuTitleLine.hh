#pragma once

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class MenuTitleLine {
public:
    class Title {
    public:
        enum {
            SelectCourse = 0x4,
            SelectMap = 0x7,
            Count = 0xb,
        };

    private:
        Title();
    };

    void drop(u32 title);
    void drop(const char *title);
    void lift();
    void draw(const J2DGraphContext *graphContext);
    void REPLACED(calc)();
    REPLACE void calc();

    static MenuTitleLine *Create(JKRArchive *archive, JKRHeap *heap);
    static void Destroy();
    static MenuTitleLine *Instance();

private:
    MenuTitleLine(JKRArchive *archive);
    ~MenuTitleLine();

    u32 m_state;
    u32 m_title;
    J2DScreen m_screen;
    J2DPicture *m_titlePictures[2];
    u8 _128[0x160 - 0x128];

    static MenuTitleLine *s_instance;
};
size_assert(MenuTitleLine, 0x160);
