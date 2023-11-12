#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>

class SceneMapSelect : public Scene {
public:
    SceneMapSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneMapSelect();
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneMapSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void moveUp();
    void moveDown();
    void scrollUp();
    void scrollDown();
    void selectIn();
    void selectOut();
    void select();
    void spin();
    void nextScene();
    void nextBattle();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateMoveUp();
    void stateMoveDown();
    void stateScrollUp();
    void stateScrollDown();
    void stateSelect();
    void stateSelectIn();
    void stateSelectOut();
    void stateSpin();
    void stateNextScene();
    void stateNextBattle();

    void refreshMaps();
    void showMaps(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    State m_state;
    u32 m_mapCount;
    u32 m_mapIndex;
    u32 m_rowCount;
    u32 m_rowIndex;
    u32 m_spinFrame;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_gridScreen;
    Array<J2DScreen, 9> m_mapScreens;
    J2DScreen m_arrowScreen;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_selectAnmTransform;
    J2DAnmBase *m_okAnmTextureSRTKey;
    J2DAnmBase *m_okAnmColor;
    J2DAnmBase *m_gridAnmTransform;
    Array<J2DAnmBase *, 9> m_mapAnmTransforms;
    Array<J2DAnmBase *, 9> m_highlightAnmTransforms;
    Array<J2DAnmBase *, 9> m_highlightAnmColors;
    Array<J2DAnmBase *, 9> m_thumbnailAnmTevRegKeys;
    Array<J2DAnmBase *, 9> m_nameAnmTevRegKeys;
    J2DAnmBase *m_arrowAnmTransform;
    u8 m_mainAnmTransformFrame;
    u8 m_selectAnmTransformFrame;
    u8 m_okAnmTextureSRTKeyFrame;
    u8 m_okAnmColorFrame;
    u8 m_gridAnmTransformFrame;
    Array<u8, 9> m_mapAnmTransformFrames;
    Array<u8, 9> m_highlightAnmTransformFrames;
    Array<u8, 9> m_highlightAnmColorFrames;
    Array<u8, 9> m_thumbnailAnmTevRegKeyFrames;
    Array<u8, 9> m_nameAnmTevRegKeyFrames;
    u8 m_arrowAnmTransformFrame;
    Array<u8, 9> m_mapAlphas;
    Array<u8, 2> m_arrowAlphas;
};
