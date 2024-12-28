#pragma once

#include "game/Scene.hh"

class SceneCoursePoll : public Scene {
public:
    SceneCoursePoll(JKRArchive *archive, JKRHeap *heap);
    ~SceneCoursePoll() override;
    void init() override;
    void draw() override;
    void calc() override;
};
