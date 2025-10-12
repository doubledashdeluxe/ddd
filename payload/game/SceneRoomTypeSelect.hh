#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>

class SceneRoomTypeSelect
    : public Scene
    , private ClientReadHandler {
public:
    SceneRoomTypeSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneRoomTypeSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        MaxRoomTypeCount = 3,
    };

    typedef void (SceneRoomTypeSelect::*State)();

    bool clientStateServer(const ClientStateServerReadInfo &readInfo) override;
    bool clientStateMode(const ClientStateModeReadInfo &readInfo) override;
    bool clientStatePack(const ClientStatePackReadInfo &readInfo) override;
    void clientStateError() override;

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    State m_state;
    u32 m_padCount;
    u32 m_statusCount;
    u32 m_roomTypeCount;
    u32 m_roomTypeIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_tandemScreen;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_mainAnmTextureSRTKey;
    J2DAnmBase *m_mainAnmColor;
    Array<J2DAnmBase *, MaxRoomTypeCount> m_roomTypeAnmTevRegKeys;
    Array<J2DAnmBase *, MaxRoomTypeCount> m_cursorAnmTransforms;
    Array<J2DAnmBase *, MaxRoomTypeCount> m_nameAnmTransforms;
    Array<J2DAnmBase *, MaxRoomTypeCount> m_circleAnmTransforms;
    J2DAnmBase *m_tandemAnmTransform;
    J2DAnmBase *m_tandemCircleAnmTransform;
    u8 m_mainAnmTransformFrame;
    u8 m_mainAnmTextureSRTKeyFrame;
    u8 m_mainAnmColorFrame;
    Array<u8, MaxRoomTypeCount> m_roomTypeAnmTevRegKeyFrames;
    Array<u8, MaxRoomTypeCount> m_cursorAnmTransformFrames;
    Array<u8, MaxRoomTypeCount> m_nameAnmTransformFrames;
    Array<u8, MaxRoomTypeCount> m_circleAnmTransformFrames;
    u8 m_tandemAnmTransformFrame;
    u8 m_tandemCircleAnmTransformFrame;
};
