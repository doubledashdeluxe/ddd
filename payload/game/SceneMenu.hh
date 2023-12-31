#pragma once

#include "game/Scene.hh"

#include <payload/Replace.hh>

class SceneMenu : public Scene {
public:
    SceneMenu(JKRArchive *archive, JKRHeap *heap);
    ~SceneMenu() override;
    void REPLACED(init)();
    REPLACE void init() override;
    void draw() override;
    void calc() override;

private:
    void REPLACED(reset)();
    REPLACE void reset();
    void REPLACED(setRaceData)();
    REPLACE void setRaceData();

    u8 _000c[0x20f0 - 0x000c];
    u32 m_nextScene;
    u8 _20f4[0x22c0 - 0x20f4];
};
size_assert(SceneMenu, 0x22c0);
