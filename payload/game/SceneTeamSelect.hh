#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>

class SceneTeamSelect
    : public Scene
    , private ClientReadHandler {
public:
    SceneTeamSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneTeamSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        MaxEntryCount = MaxRoomKartCount + 1,
    };

    typedef void (SceneTeamSelect::*State)();

    bool clientStateRoom(const ClientStateRoomReadInfo &readInfo) override;
    bool clientStateTeam(const ClientStateTeamReadInfo &readInfo) override;
    void clientStateError() override;

    void slideIn();
    void slideOut();
    void idle();
    void spin();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateSpin();
    void stateNextScene();

    State m_state;
    bool m_ok;
    bool m_isHost;
    bool m_canContinue;
    u32 m_kartCount;
    Array<u8, MaxRoomKartCount> m_teams;
    u32 m_entryIndex;
    u32 m_teamCount;
    u32 m_spinFrame;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_modeScreen;
    Array<J2DScreen, MaxEntryCount> m_entryScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_modeAnmTransform;
    J2DAnmBase *m_selectAnmTransform;
    Array<J2DAnmBase *, MaxEntryCount> m_entryAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLeftAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLeftAnmTevRegKeys;
    Array<J2DAnmBase *, MaxEntryCount> m_entryRightAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryRightAnmTevRegKeys;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLoopAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLoopAnmTextureSRTKeys;
    u8 m_mainAnmTransformFrame;
    u8 m_modeAnmTransformFrame;
    u8 m_selectAnmTransformFrame;
    Array<u8, MaxEntryCount> m_entryAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLeftAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLeftAnmTevRegKeyFrames;
    Array<u8, MaxEntryCount> m_entryRightAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryRightAnmTevRegKeyFrames;
    Array<u8, MaxEntryCount> m_entryLoopAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLoopAnmTextureSRTKeyFrames;
};
