#pragma once

#include <jsystem/J2DScreen.hh>

class SceneTitleLine {
public:
    SceneTitleLine(JKRArchive *archive, JKRHeap *heap, const char *file);
    ~SceneTitleLine();

    void init(s32 index);
    void draw(J2DGraphContext *graphContext);
    void calcAnm(s32 direction);
    void calcMatAnm();

private:
    u8 _00[0x3c - 0x00];
};
size_assert(SceneTitleLine, 0x3c);
