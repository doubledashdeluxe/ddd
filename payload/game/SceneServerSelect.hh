#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>
#include <payload/online/ClientReadHandler.hh>

class SceneServerSelect
    : public Scene
    , private ClientReadHandler {
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
    void slideIn();
    void slideOut();
    void idle();
    void scrollUp();
    void scrollDown();
    void nextScene();

    void stateWait();
    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateScrollUp();
    void stateScrollDown();
    void stateNextScene();

    void refreshServers();
    void showServers(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    State m_state;
    u32 m_serverCount;
    u32 m_serverIndex;
    u32 m_rowIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, 6> m_serverScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_arrowAnmTransform;
    Array<J2DAnmBase *, 6> m_serverAnmTransforms;
    u8 m_mainAnmTransformFrame;
    u8 m_arrowAnmTransformFrame;
    Array<u8, 6> m_serverAnmTransformFrames;
    Array<u8, 2> m_arrowAlphas;
    Array<u8, 6> m_serverAlphas;
};
