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

class SceneCoursePoll : public Scene {
public:
    SceneCoursePoll(JKRArchive *archive, JKRHeap *heap);
    ~SceneCoursePoll() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        MaxPlayerCount = 8,
    };

    typedef void (SceneCoursePoll::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void spin();
    void select();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateSpin();
    void stateSelect();
    void stateNextScene();

    void refreshCourses();

    void *load();
    bool load(const Array<u32, MaxPlayerCount> &courseIndices);

    static void *Load(void *param);

    JKRHeap *m_heap;
    State m_state;
    u32 m_courseCount;
    u32 m_playerCount;
    u32 m_playerIndex;
    u32 m_nameCount;
    u32 m_spinFrame;
    u32 m_selectFrame;
    J2DScreen m_mainScreen;
    J2DScreen m_gridScreen;
    Array<J2DScreen, MaxPlayerCount> m_courseScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_gridAnmTransform;
    Array<J2DAnmBase *, MaxPlayerCount> m_courseAnmTransforms;
    Array<J2DAnmBase *, MaxPlayerCount> m_playerNameAnmTransforms;
    Array<J2DAnmBase *, MaxPlayerCount> m_highlightAnmTransforms;
    Array<J2DAnmBase *, MaxPlayerCount> m_highlightAnmColors;
    Array<J2DAnmBase *, MaxPlayerCount> m_thumbnailAnmTevRegKeys;
    J2DAnmBase *m_courseNameAnmTransform;
    Array<J2DAnmBase *, MaxPlayerCount> m_courseNameAnmTevRegKeys;
    u8 m_mainAnmTransformFrame;
    u8 m_gridAnmTransformFrame;
    Array<u8, MaxPlayerCount> m_courseAnmTransformFrames;
    Array<u8, MaxPlayerCount> m_playerNameAnmTransformFrames;
    Array<u8, MaxPlayerCount> m_highlightAnmTransformFrames;
    Array<u8, MaxPlayerCount> m_highlightAnmColorFrames;
    Array<u8, MaxPlayerCount> m_thumbnailAnmTevRegKeyFrames;
    u8 m_courseNameAnmTransformFrame;
    Array<u8, MaxPlayerCount> m_courseNameAnmTevRegKeyFrames;
    u8 m_playerCountAlpha;
    Array<u8, MaxPlayerCount> m_courseAlphas;
    Mutex m_mutex;
    Array<u32, MaxPlayerCount> m_courseIndices;
    Array<UniquePtr<ResTIMG>, MaxPlayerCount> m_thumbnails;
    Array<UniquePtr<ResTIMG>, MaxPlayerCount> m_nameImages;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    UniquePtr<u8[]> m_loadStack;
    OSThread m_loadThread;
};
