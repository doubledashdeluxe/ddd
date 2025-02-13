#pragma once

#include "game/Scene.hh"

class SceneRoomCodeEnter : public Scene {
public:
    SceneRoomCodeEnter(JKRArchive *archive, JKRHeap *heap);
    ~SceneRoomCodeEnter() override;
    void init() override;
    void draw() override;
    void calc() override;
};
