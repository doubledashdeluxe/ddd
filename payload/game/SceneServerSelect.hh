#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <payload/SlidingText.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>

class SceneServerSelect
    : public Scene
    , private ClientReadHandler {
public:
    SceneServerSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneServerSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    class DescText : public SlidingText {
    public:
        DescText(SceneServerSelect &scene, u32 descIndex);
        ~DescText();

    private:
        const char *getPart(u32 partIndex) override;
        void setAnmTransformFrame(u8 anmTransformFrame) override;
        void setAlpha(u8 alpha) override;

        SceneServerSelect &m_scene;
        u32 m_descIndex;
    };

    typedef void (SceneServerSelect::*State)();

    bool clientStateIdle() override;
    bool clientStateServer() override;
    void clientStateError() override;

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

    void refreshServers();
    void showServers(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    State m_state;
    u32 m_serverCount;
    u32 m_serverIndex;
    u32 m_rowIndex;
    u64 m_descOffset;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, 6> m_serverScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_arrowAnmTransform;
    Array<J2DAnmBase *, 6> m_serverAnmTransforms;
    Array<J2DAnmBase *, 6> m_descAnmTransforms;
    u8 m_mainAnmTransformFrame;
    u8 m_arrowAnmTransformFrame;
    Array<u8, 6> m_serverAnmTransformFrames;
    Array<u8, 6> m_descAnmTransformFrames;
    Array<u8, 2> m_arrowAlphas;
    Array<u8, 6> m_serverAlphas;
    Array<u8, 6> m_descAlphas;
};
