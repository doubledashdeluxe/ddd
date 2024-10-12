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
    u64 m_descOffset;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_modeScreen;
    Array<J2DScreen, 6> m_packScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_arrowAnmTransform;
    J2DAnmBase *m_modeAnmTransform;
    Array<J2DAnmBase *, 6> m_packAnmTransforms;
    Array<J2DAnmBase *, 6> m_descAnmTransforms;
    u8 m_mainAnmTransformFrame;
    u8 m_arrowAnmTransformFrame;
    u8 m_modeAnmTransformFrame;
    Array<u8, 6> m_packAnmTransformFrames;
    Array<u8, 6> m_descAnmTransformFrames;
    Array<u8, 2> m_arrowAlphas;
    Array<u8, 6> m_packAlphas;
    Array<u8, 6> m_descAlphas;
};
