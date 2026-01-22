#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStateRoomWriteInfo.hh>

class ScenePersonalRoom
    : public Scene
    , private ClientReadHandler {
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
        MaxOptionCount = 7,
        MaxEntryCount = MaxOptionCount + 1,
    };

    typedef void (ScenePersonalRoom::*State)();

    bool clientStateMode(const ClientStateModeReadInfo &readInfo) override;
    bool clientStatePack(const ClientStatePackReadInfo &readInfo) override;
    bool clientStateRoom(const ClientStateRoomReadInfo &readInfo) override;
    void clientStateError() override;

    void wait();
    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateWait();
    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    bool m_ok;
    bool m_isReady;
    bool m_isSearch;
    bool m_isHost;
    bool m_isRace;
    bool m_canContinue;
    bool m_continuing;
    u32 m_charCount;
    Array<u8, MaxCharCount> m_chars;
    bool m_revealCode;
    u32 m_kartCount;
    u32 m_optionCount;
    Array<u8, MaxOptionCount> m_options;
    Array<u8, MaxOptionCount> m_values;
    u32 m_entryIndex;
    ClientStateRoomWriteInfo m_writeInfo;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, MaxCharCount> m_charScreens;
    Array<J2DScreen, MaxRoomKartCount> m_playerScreens;
    Array<J2DScreen, MaxEntryCount> m_entryScreens;
    J2DAnmBase *m_mainAnmTransform;
    Array<J2DAnmBase *, SwitchCount> m_switchAnmTransforms;
    Array<J2DAnmBase *, SwitchCount> m_switchAnmTexPatterns;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTextureSRTKeys;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTexPatterns;
    Array<J2DAnmBase *, MaxRoomKartCount> m_playerAnmTransforms;
    Array<J2DAnmBase *, MaxRoomKartCount> m_playerNameAnmTransforms;
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
    Array<u8, MaxRoomKartCount> m_playerAnmTransformFrames;
    Array<u8, MaxRoomKartCount> m_playerNameAnmTransformFrames;
    u8 m_playerCircleAnmTransformFrame;
    Array<u8, MaxEntryCount> m_entryAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLeftAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLeftAnmTevRegKeyFrames;
    Array<u8, MaxEntryCount> m_entryRightAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryRightAnmTevRegKeyFrames;
    Array<u8, MaxEntryCount> m_entryLoopAnmTransformFrames;
    Array<u8, MaxEntryCount> m_entryLoopAnmTextureSRTKeyFrames;
};
