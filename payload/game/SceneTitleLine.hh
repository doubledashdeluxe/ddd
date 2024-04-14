#pragma once

#include <jsystem/J2DScreen.hh>

class SceneTitleLine {
public:
    SceneTitleLine(JKRArchive *archive, JKRHeap *heap, const char *file);
    ~SceneTitleLine();

    void REPLACED(init)(s32 index);
    REPLACE void init(s32 index);
    void draw(J2DGraphContext *graphContext);
    void calcAnm(s32 direction);
    void calcMatAnm();

private:
    J2DScreen *m_screen;
    u8 _04[0x3c - 0x04];
};
size_assert(SceneTitleLine, 0x3c);
