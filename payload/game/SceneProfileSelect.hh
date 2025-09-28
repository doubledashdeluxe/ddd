#pragma once

#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class SceneProfileSelect : public Scene {
public:
    SceneProfileSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneProfileSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    enum {
        ColCount = 4,
        RowCount = 4,
        ProfileCount = RowCount * ColCount,
    };

    typedef void (SceneProfileSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateNextScene();

    u32 countSelectedProfiles() const;
    void move(u32 padIndex, s32 x, s32 y);
    bool move(u32 padIndex, u32 profileIndex);
    void selectProfile(u32 padIndex);
    void deselectProfile(u32 padIndex);

    State m_state;
    u32 m_padCount;
    Array<u32, 4> m_profileIndices;
    u32 m_selectedFlags;
    u32 m_nextScene;
    J2DScreen m_mainScreen;
    Array<J2DScreen, ProfileCount> m_profileScreens;
    Array<J2DAnmBase *, ProfileCount> m_profileAnmTransforms;
    Array<J2DAnmBase *, ProfileCount> m_profileAnmTextureSRTKeys;
    Array<J2DAnmBase *, ProfileCount> m_windowAnmTevRegKeys;
    Array<J2DAnmBase *, ProfileCount> m_outlineAnmTevRegKeys;
    Array<J2DAnmBase *, ProfileCount> m_padAnmColors;
    Array<J2DAnmBase *, ProfileCount> m_nameAnmColors;
    Array<J2DAnmBase *, ProfileCount> m_windowAnmColors;
    Array<u8, ProfileCount> m_profileAnmTransformFrames;
    Array<u8, ProfileCount> m_profileAnmTextureSRTKeyFrames;
    Array<u8, ProfileCount> m_windowAnmTevRegKeyFrames;
    Array<u8, ProfileCount> m_outlineAnmTevRegKeyFrames;
    Array<u8, ProfileCount> m_padAnmColorFrames;
    Array<u8, ProfileCount> m_nameAnmColorFrames;
    Array<u8, ProfileCount> m_windowAnmColorFrames;
};
