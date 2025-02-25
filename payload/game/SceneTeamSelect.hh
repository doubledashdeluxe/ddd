#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>

class SceneTeamSelect : public Scene {
public:
    SceneTeamSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneTeamSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        MaxPlayerCount = 8,
        MaxEntryCount = MaxPlayerCount + 1,
    };

    typedef void (SceneTeamSelect::*State)();

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
    u32 m_playerCount;
    Array<u8, MaxPlayerCount> m_teams;
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
