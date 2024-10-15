#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <common/UniquePtr.hh>
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <jsystem/J2DScreen.hh>
#include <jsystem/ResTIMG.hh>
#include <payload/Mutex.hh>

class SceneMapSelect : public Scene {
public:
    SceneMapSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneMapSelect() override;
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

    void refreshSpin();
    void refreshMaps();
    void refreshMaps(const Array<u32, 12> &nextMapIndices);
    void showMaps(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    void *load();
    bool load(const Array<u32, 12> &nextMapIndices);
    UniquePtr<ResTIMG> &findTexture(Array<UniquePtr<ResTIMG>, 12> &textures,
            const Array<u32, 12> &nextMapIndices, u32 mapIndex);

    static void *Load(void *param);

    JKRHeap *m_heap;
    State m_state;
    u32 m_mapCount;
    u32 m_mapIndex;
    u32 m_rowCount;
    u32 m_rowIndex;
    u32 m_spinFrame;
    u32 m_spinMapIndex;
    u32 m_spinRowIndex;
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
    J2DAnmBase *m_nameAnmTransform;
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
    u8 m_nameAnmTransformFrame;
    Array<u8, 9> m_nameAnmTevRegKeyFrames;
    u8 m_arrowAnmTransformFrame;
    Array<u8, 9> m_mapAlphas;
    Array<u8, 2> m_arrowAlphas;
    Mutex m_mutex;
    Array<u32, 12> m_nextMapIndices;
    Array<u32, 12> m_currMapIndices;
    Array<UniquePtr<ResTIMG>, 12> m_thumbnails;
    Array<UniquePtr<ResTIMG>, 12> m_nameImages;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    Array<u8, 64 * 1024> m_loadStack;
    OSThread m_loadThread;
};
