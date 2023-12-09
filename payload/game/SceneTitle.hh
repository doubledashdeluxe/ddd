#pragma once

#include "game/Scene.hh"

#include <payload/Replace.hh>

class SceneTitle : public Scene {
public:
    SceneTitle(JKRArchive *archive, JKRHeap *heap);
    ~SceneTitle() override;
    void REPLACED(init)();
    REPLACE void init() override;
    void draw() override;
    void calc() override;

private:
    u8 _00c[0x174 - 0x00c];
};
size_assert(SceneTitle, 0x174);
