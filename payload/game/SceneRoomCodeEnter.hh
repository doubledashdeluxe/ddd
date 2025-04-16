#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class SceneRoomCodeEnter : public Scene {
public:
    SceneRoomCodeEnter(JKRArchive *archive, JKRHeap *heap);
    ~SceneRoomCodeEnter() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        MaxCharCount = 20,
    };

    typedef void (SceneRoomCodeEnter::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void selectIn();
    void selectOut();
    void select();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateSelectIn();
    void stateSelectOut();
    void stateSelect();
    void stateNextScene();

    State m_state;
    u32 m_padCount;
    u32 m_statusCount;
    u32 m_charCount;
    Array<u8, MaxCharCount> m_chars;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_tandemScreen;
    Array<J2DScreen, MaxCharCount> m_charScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_selectAnmTransform;
    J2DAnmBase *m_tandemAnmTransform;
    J2DAnmBase *m_tandemCircleAnmTransform;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTransforms;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTextureSRTKeys;
    Array<J2DAnmBase *, MaxCharCount> m_charAnmTexPatterns;
    u8 m_mainAnmTransformFrame;
    u8 m_selectAnmTransformFrame;
    u8 m_tandemAnmTransformFrame;
    u8 m_tandemCircleAnmTransformFrame;
    Array<u8, MaxCharCount> m_charAnmTransformFrames;
    Array<u8, MaxCharCount> m_charAnmTextureSRTKeyFrames;
    Array<u8, MaxCharCount> m_charAnmTexPatternFrames;
    Array<u8, MaxCharCount> m_charAlphas;
};
