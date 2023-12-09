#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>

class ScenePackSelect : public Scene {
public:
    ScenePackSelect(JKRArchive *archive, JKRHeap *heap);
    ~ScenePackSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (ScenePackSelect::*State)();

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

    void refreshPacks();
    void showPacks(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    State m_state;
    u32 m_packCount;
    u32 m_packIndex;
    u32 m_rowIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_modeScreen;
    Array<J2DScreen, 4> m_countScreens;
    J2DScreen m_arrowScreen;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_mainAnmTextureSRTKey;
    Array<J2DAnmBase *, 4> m_packAnmTransforms;
    Array<J2DAnmBase *, 4> m_packAnmColors;
    Array<J2DAnmBase *, 4> m_packAnmTevRegKeys;
    Array<J2DAnmBase *, 4> m_circleAnmTransforms;
    J2DAnmBase *m_modeAnmTransform;
    Array<J2DAnmBase *, 4> m_countAnmTransforms;
    J2DAnmBase *m_arrowAnmTransform;
    u8 m_mainAnmTransformFrame;
    u8 m_mainAnmTextureSRTKeyFrame;
    Array<u8, 4> m_packAnmTransformFrames;
    Array<u8, 4> m_packAnmColorFrames;
    Array<u8, 4> m_packAnmTevRegKeyFrames;
    Array<u8, 4> m_circleAnmTransformFrames;
    u8 m_modeAnmTransformFrame;
    u8 m_arrowAnmTransformFrame;
    Array<u8, 4> m_packAlphas;
    Array<u8, 2> m_arrowAlphas;
};
