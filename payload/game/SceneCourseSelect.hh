#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <payload/Replace.hh>

class SceneCourseSelect : public Scene {
public:
    SceneCourseSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneCourseSelect() override;
    void REPLACED(init)();
    REPLACE void init() override;
    void draw() override;
    void calc() override;

private:
    u8 _00c[0x27c - 0x00c];
    J2DScreen *m_underScreen;
    u8 _280[0x398 - 0x280];
};
size_assert(SceneCourseSelect, 0x398);
