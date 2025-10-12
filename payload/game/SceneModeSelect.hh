#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <payload/SlidingText.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>

class SceneModeSelect
    : public Scene
    , private ClientReadHandler {
public:
    enum {
        ModeCount = 5,
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

    void refreshModes();

    static const char *String(u32 index);

    State m_state;
    u32 m_roomType;
    u32 m_modeIndex;
    Array<Array<char, 80>, ModeCount> m_descs;
    Array<u64, ModeCount> m_descOffsets;
    Array<Array<char, 4>, ModeCount> m_playerCounts;
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
