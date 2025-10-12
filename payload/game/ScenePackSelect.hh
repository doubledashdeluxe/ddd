#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <payload/SlidingText.hh>
#include <portable/Array.hh>
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStatePackWriteInfo.hh>

class ScenePackSelect
    : public Scene
    , private ClientReadHandler {
public:
    ScenePackSelect(JKRArchive *archive, JKRHeap *heap);
    ~ScenePackSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    class DescText : public SlidingText {
    public:
        DescText(ScenePackSelect &scene, u32 descIndex);
        ~DescText();

    private:
        const char *getPart(u32 partIndex) override;
        void setAnmTransformFrame(u8 anmTransformFrame) override;
        void setAlpha(u8 alpha) override;

        ScenePackSelect &m_scene;
        u32 m_descIndex;
    };

    typedef void (ScenePackSelect::*State)();

    bool clientStateMode(const ClientStateModeReadInfo &readInfo) override;
    bool clientStatePack(const ClientStatePackReadInfo &readInfo) override;
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

    void refreshPacks();
    void showPacks(s32 rowOffset);
    void showArrows(s32 rowOffset);
    void hideArrows();

    State m_state;
    u32 m_packCount;
    u32 m_packIndex;
    u32 m_rowIndex;
    u64 m_descOffset;
    Array<Array<char, 4>, MaxPackCount> m_playerCounts;
    ClientStatePackWriteInfo m_writeInfo;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    J2DScreen m_modeScreen;
    Array<J2DScreen, 6> m_packScreens;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_arrowAnmTransform;
    J2DAnmBase *m_modeAnmTransform;
    J2DAnmBase *m_onlineAnmTransform;
    Array<J2DAnmBase *, 6> m_packAnmTransforms;
    Array<J2DAnmBase *, 6> m_descAnmTransforms;
    u8 m_mainAnmTransformFrame;
    u8 m_arrowAnmTransformFrame;
    u8 m_modeAnmTransformFrame;
    u8 m_onlineAnmTransformFrame;
    Array<u8, 6> m_packAnmTransformFrames;
    Array<u8, 6> m_descAnmTransformFrames;
    Array<u8, 2> m_arrowAlphas;
    Array<u8, 6> m_packAlphas;
    Array<u8, 6> m_descAlphas;
};
