#pragma once

#include "game/NameEntryHelper.hh"
#include "game/Scene.hh"

#include <common/Array.hh>
#include <common/UniquePtr.hh>
#include <jsystem/J2DScreen.hh>

class SceneNameSelect : public Scene {
public:
    SceneNameSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneNameSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneNameSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void wait();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateWait();
    void stateNextScene();

    void readNames();
    void writeNames();

    State m_state;
    u32 m_padCount;
    Array<Array<char, 4>, 4> m_savedNames;
    Array<Array<char, 4>, 4> m_unsavedNames;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_padCountScreen;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_nameAnmTransform;
    J2DAnmBase *m_nameCircleAnmTransform;
    J2DAnmBase *m_padCountAnmTransform;
    J2DAnmBase *m_padCountCircleAnmTransform;
    u8 m_mainAnmTransformFrame;
    u8 m_nameAnmTransformFrame;
    u8 m_nameCircleAnmTransformFrame;
    u8 m_padCountAnmTransformFrame;
    u8 m_padCountCircleAnmTransformFrame;
    Array<UniquePtr<NameEntryHelper>, 4> m_nameEntryHelpers;
};
