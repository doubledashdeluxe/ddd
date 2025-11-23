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
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStatePollWriteInfo.hh>

class SceneCoursePoll
    : public Scene
    , private ClientReadHandler {
public:
    SceneCoursePoll(JKRArchive *archive, JKRHeap *heap);
    ~SceneCoursePoll() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneCoursePoll::*State)();

    bool clientStatePoll(const ClientStatePollReadInfo &readInfo) override;
    void clientStateError() override;

    void slideIn();
    void slideOut();
    void idle();
    void spin();
    void select();
    void nextScene();
    void nextRace();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateSpin();
    void stateSelect();
    void stateNextScene();
    void stateNextRace();

    void refreshCourses();

    void *load();
    bool load(const Array<u32, MaxRoomKartCount> &courseIndices);

    static void *Load(void *param);

    JKRHeap *m_heap;
    State m_state;
    bool m_ok;
    u32 m_courseCount;
    u32 m_kartCount;
    u32 m_kartIndex;
    u32 m_nameCount;
    Optional<u32> m_selectedKartIndex;
    Array<u8, MaxRoomKartCount> m_courseShuffleIndices;
    u32 m_spinFrame;
    u32 m_selectFrame;
    ClientStatePollWriteInfo m_writeInfo;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_gridScreen;
    Array<J2DScreen, MaxRoomKartCount> m_courseScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_gridAnmTransform;
    Array<J2DAnmBase *, MaxRoomKartCount> m_courseAnmTransforms;
    Array<J2DAnmBase *, MaxRoomKartCount> m_playerNameAnmTransforms;
    Array<J2DAnmBase *, MaxRoomKartCount> m_highlightAnmTransforms;
    Array<J2DAnmBase *, MaxRoomKartCount> m_highlightAnmColors;
    Array<J2DAnmBase *, MaxRoomKartCount> m_thumbnailAnmTevRegKeys;
    J2DAnmBase *m_courseNameAnmTransform;
    Array<J2DAnmBase *, MaxRoomKartCount> m_courseNameAnmTevRegKeys;
    u8 m_mainAnmTransformFrame;
    u8 m_gridAnmTransformFrame;
    Array<u8, MaxRoomKartCount> m_courseAnmTransformFrames;
    Array<u8, MaxRoomKartCount> m_playerNameAnmTransformFrames;
    Array<u8, MaxRoomKartCount> m_highlightAnmTransformFrames;
    Array<u8, MaxRoomKartCount> m_highlightAnmColorFrames;
    Array<u8, MaxRoomKartCount> m_thumbnailAnmTevRegKeyFrames;
    u8 m_courseNameAnmTransformFrame;
    Array<u8, MaxRoomKartCount> m_courseNameAnmTevRegKeyFrames;
    u8 m_kartCountAlpha;
    Array<u8, MaxRoomKartCount> m_courseAlphas;
    Mutex m_mutex;
    Array<u32, MaxRoomKartCount> m_courseIndices;
    Array<UniquePtr<ResTIMG>, MaxRoomKartCount> m_thumbnails;
    Array<UniquePtr<ResTIMG>, MaxRoomKartCount> m_nameImages;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    UniquePtr<u8[]> m_loadStack;
    OSThread m_loadThread;
};
