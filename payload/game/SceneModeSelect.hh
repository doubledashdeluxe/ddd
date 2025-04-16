#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <payload/SlidingText.hh>
#include <portable/Array.hh>

class SceneModeSelect : public Scene {
public:
    enum {
        ModeCount = 4,
    };

    SceneModeSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneModeSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    class DescText : public SlidingText {
    public:
        DescText(SceneModeSelect &scene, u32 descIndex);
        ~DescText();

    private:
        const char *getPart(u32 partIndex) override;
        void setAnmTransformFrame(u8 anmTransformFrame) override;
        void setAlpha(u8 alpha) override;

        SceneModeSelect &m_scene;
        u32 m_descIndex;
    };

    typedef void (SceneModeSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    void refreshModes();

    State m_state;
    u32 m_modeIndex;
    u64 m_descOffset;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, ModeCount> m_modeScreens;
    J2DAnmBase *m_mainAnmTransform;
    Array<J2DAnmBase *, ModeCount> m_modeAnmTransforms;
    Array<J2DAnmBase *, ModeCount> m_descAnmTransforms;
    u8 m_mainAnmTransformFrame;
    Array<u8, ModeCount> m_modeAnmTransformFrames;
    Array<u8, ModeCount> m_descAnmTransformFrames;
    Array<u8, ModeCount> m_descAlphas;

    static const Array<u32, ModeCount> Modes;
};
