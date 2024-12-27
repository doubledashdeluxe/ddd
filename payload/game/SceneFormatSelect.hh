#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>

class SceneFormatSelect : public Scene {
public:
    SceneFormatSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneFormatSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        FormatCount = 3,
    };

    typedef void (SceneFormatSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    void refreshFormats();

    State m_state;
    u32 m_formatIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_modeScreen;
    Array<J2DScreen, FormatCount> m_playerCountScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_mainAnmTextureSRTKey;
    J2DAnmBase *m_mainAnmColor;
    J2DAnmBase *m_modeAnmTransform;
    Array<J2DAnmBase *, FormatCount> m_formatAnmTevRegKeys;
    Array<J2DAnmBase *, FormatCount> m_cursorAnmTransforms;
    Array<J2DAnmBase *, FormatCount> m_nameAnmTransforms;
    Array<J2DAnmBase *, FormatCount> m_circleAnmTransforms;
    Array<J2DAnmBase *, FormatCount> m_playerCountAnmTevRegKeys;
    u8 m_mainAnmTransformFrame;
    u8 m_mainAnmTextureSRTKeyFrame;
    u8 m_mainAnmColorFrame;
    u8 m_modeAnmTransformFrame;
    Array<u8, FormatCount> m_formatAnmTevRegKeyFrames;
    Array<u8, FormatCount> m_cursorAnmTransformFrames;
    Array<u8, FormatCount> m_nameAnmTransformFrames;
    Array<u8, FormatCount> m_circleAnmTransformFrames;
    Array<u8, FormatCount> m_playerCountAnmTevRegKeyFrames;
};
