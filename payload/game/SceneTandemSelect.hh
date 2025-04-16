#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class SceneTandemSelect : public Scene {
public:
    SceneTandemSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneTandemSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneTandemSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    static u32 GetStatusCount(u32 padCount, u32 partitionIndex);
    static u32 GetFirstStatusIndex(u32 padCount, u32 partitionIndex);

    State m_state;
    u32 m_padCount;
    u32 m_partitionCount;
    u32 m_partitionIndex;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_padCountScreen;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_mainAnmTextureSRTKey;
    J2DAnmBase *m_mainAnmColor;
    Array<J2DAnmBase *, 3> m_partitionAnmTransforms;
    Array<J2DAnmBase *, 3> m_partitionAnmTextureSRTKeys;
    Array<J2DAnmBase *, 3> m_partitionAnmTevRegKeys;
    Array<J2DAnmBase *, 3> m_partitionCircleAnmTransforms;
    Array<J2DAnmBase *, 3> m_partitionWindowAnmTransforms;
    J2DAnmBase *m_padCountAnmTransform;
    J2DAnmBase *m_padCountCircleAnmTransform;
    u8 m_mainAnmTransformFrame;
    u8 m_mainAnmTextureSRTKeyFrame;
    u8 m_mainAnmColorFrame;
    Array<u8, 3> m_partitionAnmTransformFrames;
    Array<u8, 3> m_partitionAnmTextureSRTKeyFrames;
    Array<u8, 3> m_partitionAnmTevRegKeyFrames;
    Array<u8, 3> m_partitionCircleAnmTransformFrames;
    Array<u8, 3> m_partitionWindowAnmTransformFrames;
    u8 m_padCountAnmTransformFrame;
    u8 m_padCountCircleAnmTransformFrame;
};
