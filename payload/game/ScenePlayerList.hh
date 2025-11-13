#pragma once

#include "game/Scene.hh"

#include <formats/Online.hh>
#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class ScenePlayerList : public Scene {
public:
    ScenePlayerList(JKRArchive *archive, JKRHeap *heap);
    ~ScenePlayerList() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (ScenePlayerList::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, MaxRoomKartCount> m_kartScreens;
    J2DAnmBase *m_mainAnmTransform;
    u8 m_mainAnmTransformFrame;
};
