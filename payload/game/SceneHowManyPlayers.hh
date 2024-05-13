#pragma once

#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>

class SceneHowManyPlayers : public Scene {
public:
    SceneHowManyPlayers(JKRArchive *archive, JKRHeap *heap);
    ~SceneHowManyPlayers() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneHowManyPlayers::*State)();

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
    u32 m_nextScene;
    J2DScreen m_screen;
    J2DAnmBase *m_anmTransform;
    J2DAnmBase *m_anmTextureSRTKey;
    J2DAnmBase *m_anmColor;
    Array<J2DAnmBase *, 4> m_countAnmTransforms;
    Array<J2DAnmBase *, 4> m_countAnmTevRegKeys;
    Array<J2DAnmBase *, 4> m_circleAnmTransforms;
    u8 m_anmTransformFrame;
    u8 m_anmTextureSRTKeyFrame;
    u8 m_anmColorFrame;
    Array<u8, 4> m_countAnmTransformFrames;
    Array<u8, 4> m_countAnmTevRegKeyFrames;
    Array<u8, 4> m_circleAnmTransformFrames;
};
