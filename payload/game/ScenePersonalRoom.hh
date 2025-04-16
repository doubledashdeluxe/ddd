#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class ScenePersonalRoom : public Scene {
public:
    ScenePersonalRoom(JKRArchive *archive, JKRHeap *heap);
    ~ScenePersonalRoom() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        SwitchCount = 2,
        MaxCharCount = 20,
        MaxPlayerCount = 8,
        MaxOptionCount = 7,
        MaxEntryCount = MaxOptionCount + 1,
    };

    typedef void (ScenePersonalRoom::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    u32 m_charCount;
    Array<u8, MaxCharCount> m_chars;
    bool m_revealCode;
    u32 m_optionCount;
    Array<u8, MaxOptionCount> m_options;
    Array<u8, MaxOptionCount> m_values;
    u32 m_entryIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, MaxCharCount> m_charScreens;
    Array<J2DScreen, MaxPlayerCount> m_playerScreens;
    Array<J2DScreen, MaxEntryCount> m_entryScreens;
    J2DAnmBase *m_mainAnmTransform;
    Array<J2DAnmBase *, SwitchCount> m_switchAnmTransforms;
    Array<J2DAnmBase *, SwitchCount> m_switchAnmTexPatterns;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTextureSRTKeys;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTexPatterns;
    Array<J2DAnmBase *, MaxPlayerCount> m_playerAnmTransforms;
    Array<J2DAnmBase *, MaxPlayerCount> m_playerNameAnmTransforms;
    J2DAnmBase *m_playerCircleAnmTransform;
    Array<J2DAnmBase *, MaxEntryCount> m_entryAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLeftAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLeftAnmTevRegKeys;
    Array<J2DAnmBase *, MaxEntryCount> m_entryRightAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryRightAnmTevRegKeys;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLoopAnmTransforms;
    Array<J2DAnmBase *, MaxEntryCount> m_entryLoopAnmTextureSRTKeys;
    u8 m_mainAnmTransformFrame;
    Array<u8, SwitchCount> m_switchAnmTransformFrames;
    Array<u8, SwitchCount> m_switchAnmTexPatternFrames;
    Array<u8, MaxCharCount> m_charAnmTextureSRTKeyFrames;
    Array<u8, MaxCharCount> m_charAnmTexPatternFrames;
    Array<u8, MaxPlayerCount> m_playerAnmTransformFrames;
    Array<u8, MaxPlayerCount> m_playerNameAnmTransformFrames;
    u8 m_playerCircleAnmTransformFrame;
    Array<u8, MaxEntryCount> m_entryAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLeftAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLeftAnmTevRegKeyFrames;
    Array<u8, MaxEntryCount> m_entryRightAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryRightAnmTevRegKeyFrames;
    Array<u8, MaxEntryCount> m_entryLoopAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLoopAnmTextureSRTKeyFrames;
};
