#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStatePollWriteInfo.hh>

class ScenePlayerList
    : public Scene
    , private ClientReadHandler {
public:
    ScenePlayerList(JKRArchive *archive, JKRHeap *heap);
    ~ScenePlayerList() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (ScenePlayerList::*State)();

    bool clientStateRoom(const ClientStateRoomReadInfo &readInfo) override;
    bool clientStateTeam(const ClientStateTeamReadInfo &readInfo) override;
    bool clientStatePoll(const ClientStatePollReadInfo &readInfo) override;
    void clientStateError() override;

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    bool m_ok;
    ClientStatePollWriteInfo m_writeInfo;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, MaxRoomKartCount> m_kartScreens;
    J2DAnmBase *m_mainAnmTransform;
    u8 m_mainAnmTransformFrame;
};
