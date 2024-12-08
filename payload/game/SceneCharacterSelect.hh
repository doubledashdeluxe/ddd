#pragma once

#include "game/CharacterID.hh"
#include "game/KartID.hh"
#include "game/Scene.hh"

#include <common/Array.hh>
#include <jsystem/J2DScreen.hh>

class SceneCharacterSelect : public Scene {
public:
    SceneCharacterSelect(JKRArchive *archive, JKRHeap *heap);
    ~SceneCharacterSelect() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    typedef void (SceneCharacterSelect::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void spin();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateSpin();
    void stateNextScene();

    u32 countSelectedCharacters() const;
    u32 countSelectedKarts() const;
    void move(u32 padIndex, s32 x, s32 y);
    bool move(u32 padIndex, u32 characterIndex);
    void selectCharacter(u32 padIndex);
    void deselectCharacter(u32 padIndex);
    void changeKart(u32 statusIndex, s32 x);
    void selectKart(u32 statusIndex);
    void deselectKart(u32 statusIndex);

    static u32 GetCharacterIndex(u32 characterID);

    State m_state;
    u32 m_padCount;
    u32 m_statusCount;
    Array<u32, 4> m_statuses;
    Array<Array<u32, 2>, 4> m_pads;
    Array<u32, 4> m_characterIndices;
    Array<u32, 4> m_kartIndices;
    Array<Array<u32, 2>, 4> m_characterIDs;
    Array<u32, 4> m_kartIDs;
    u32 m_spinFrame;
    u32 m_nextScene;
    Array<J2DScreen, 4> m_mainScreens;
    Array<J2DScreen, CharacterID::Count / 2> m_colScreens;
    J2DScreen m_extraScreen;
    Array<J2DAnmBase *, 4> m_mainAnmTransforms;
    Array<J2DAnmBase *, 4> m_mainAnmTextureSRTKeys;
    Array<J2DAnmBase *, 2> m_mainAnmTevRegKeys;
    Array<J2DAnmBase *, 4> m_mainAnmColors;
    Array<J2DAnmBase *, 4> m_arrowAnmTransforms;
    Array<J2DAnmBase *, CharacterID::Count> m_colAnmTransforms;
    Array<J2DAnmBase *, CharacterID::Count / 2> m_colAnmTextureSRTKeys;
    Array<J2DAnmBase *, CharacterID::Count> m_windowAnmTevRegKeys;
    Array<J2DAnmBase *, CharacterID::Count> m_outlineAnmTevRegKeys;
    Array<J2DAnmBase *, CharacterID::Count> m_padAnmColors;
    Array<J2DAnmBase *, CharacterID::Count> m_characterAnmColors;
    Array<J2DAnmBase *, CharacterID::Count> m_windowAnmColors;
    Array<J2DAnmBase *, 2> m_extraAnmTransforms;
    Array<J2DAnmBase *, 2> m_extraAnmTevRegKeys;
    J2DAnmBase *m_extraAnmColor;
    u8 m_mainAnmTransformFrame;
    u8 m_mainAnmTextureSRTKeyFrame;
    u8 m_mainAnmTevRegKeyFrame;
    u8 m_mainAnmColorFrame;
    u8 m_arrowAnmTransformFrame;
    Array<u8, CharacterID::Count> m_colAnmTransformFrames;
    Array<u8, CharacterID::Count / 2> m_colAnmTextureSRTKeyFrames;
    Array<u8, CharacterID::Count> m_windowAnmTevRegKeyFrames;
    Array<u8, CharacterID::Count> m_outlineAnmTevRegKeyFrames;
    Array<u8, CharacterID::Count> m_padAnmColorFrames;
    Array<u8, CharacterID::Count> m_characterAnmColorFrames;
    Array<u8, CharacterID::Count> m_windowAnmColorFrames;
    Array<u8, 2> m_extraAnmTransformFrames;
    Array<u8, 2> m_extraAnmTevRegKeyFrames;
    u8 m_extraAnmColorFrame;
    TBox<f32> m_colBox;

    static const Array<u32, CharacterID::Count> CharacterIDs;
    static const Array<u32, KartID::Count> KartIDs;
    static const Array<const char *, CharacterID::Count> CharacterNames;
    static const Array<Array<u8, 3>, KartID::Count> KartStars;
    static const Array<u32, CharacterID::Count> CharacterSelectionSoundIDs;
    static const Array<u32, CharacterID::Count> KartSelectionSoundIDs;
};
