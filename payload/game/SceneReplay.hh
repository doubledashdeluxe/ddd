#pragma once

#include "game/Scene.hh"

extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <jsystem/J2DScreen.hh>
#include <jsystem/ResTIMG.hh>
#include <payload/Mutex.hh>
#include <portable/Array.hh>
#include <portable/UniquePtr.hh>

class SceneReplay : public Scene {
public:
    SceneReplay(JKRArchive *archive, JKRHeap *heap);
    ~SceneReplay() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneReplay::*State)();

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

    void refreshReplays();
    void showReplays(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    void *load();
    bool load(const Array<u32, 12> &nextReplayIndices);
    UniquePtr<ResTIMG> &findLogo(const Array<u32, 12> &nextReplayIndices, u32 replayIndex);

    static void *Load(void *param);

    JKRHeap *m_heap;
    State m_state;
    u32 m_replayCount;
    u32 m_replayIndex;
    u32 m_rowIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_downloadScreen;
    Array<J2DScreen, 6> m_replayScreens;
    Array<Array<J2DScreen, 8>, 6> m_playerScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_arrowAnmTransform;
    J2DAnmBase *m_downloadAnmTransform;
    J2DAnmBase *m_selectAnmTransform;
    Array<J2DAnmBase *, 6> m_replayAnmTransforms;
    Array<Array<J2DAnmBase *, 8>, 6> m_playerAnmTransforms;
    u8 m_mainAnmTransformFrame;
    u8 m_arrowAnmTransformFrame;
    u8 m_downloadAnmTransformFrame;
    u8 m_selectAnmTransformFrame;
    Array<u8, 6> m_replayAnmTransformFrames;
    Array<Array<u8, 8>, 6> m_playerAnmTransformFrames;
    Array<u8, 2> m_arrowAlphas;
    Array<u8, 6> m_replayAlphas;
    bool m_loading;
    Mutex m_mutex;
    Array<u32, 12> m_nextReplayIndices;
    Array<u32, 12> m_currReplayIndices;
    Array<UniquePtr<ResTIMG>, 12> m_logos;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    UniquePtr<u8[]> m_loadStack;
    OSThread m_loadThread;
};
