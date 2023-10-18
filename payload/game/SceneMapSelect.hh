#pragma once

#include "game/Scene.hh"

class SceneMapSelect : public Scene {
public:
    SceneMapSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneMapSelect();
    void init() override;
    void draw() override;
    void calc() override;
};
