#pragma once

#include "game/Scene.hh"

#include <payload/online/ClientReadHandler.hh>

class SceneServerSelect : public Scene, private ClientReadHandler {
public:
    SceneServerSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneServerSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneServerSelect::*State)();

    bool clientStateServer() override;
    void clientStateError() override;

    void wait();
    void idle();

    void stateWait();
    void stateIdle();

    State m_state;
};
