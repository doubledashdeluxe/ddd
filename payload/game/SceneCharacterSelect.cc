#include "SceneCharacterSelect.hh"

#include "game/CharacterSelect3D.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <common/Algorithm.hh>
#include <common/Align.hh>
extern "C" {
#include <dolphin/GXTransform.h>
#include <dolphin/OSTime.h>
}
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DOrthoGraph.hh>
#include <jsystem/JKRExpHeap.hh>

extern "C" {
#include <math.h>
#include <stdio.h>
}

SceneCharacterSelect::SceneCharacterSelect(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap), m_parentHeap(heap) {
    for (u32 i = 0; i < m_mainScreens.count(); i++) {
        Array<char, 32> file;
        snprintf(file.values(), file.count(), "select_character%u.blo", i + 1);
        m_mainScreens[i].set(file.values(), 0x1040000, m_archive);
    }
    J2DScreen colBoxScreen;
    colBoxScreen.set("select_character_cc_layout.blo", 0x1040000, m_archive);
    m_colBox.start = colBoxScreen.search("SetCha1")->getBox().start;
    m_colBox.end = colBoxScreen.search("SetCha2")->getBox().start;
    for (u32 i = 0; i < m_colScreens.count(); i++) {
        m_colScreens[i].set("select_character_cc.blo", 0x1040000, m_archive);
    }
    m_extraScreen.set("select_character_plus1.blo", 0x1040000, m_archive);

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < CharacterIDs.count(); i++) {
        J2DPicture *picture = m_colScreens[i / 2].search("CP%u", i % 2 + 1)->downcast<J2DPicture>();
        picture->changeTexture(kart2DCommon->getCharacterIcon(CharacterIDs[i]), 0);
    }

    for (u32 i = 0; i < m_mainAnmTransforms.count(); i++) {
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "select_character%u.bck", i + 1);
        m_mainAnmTransforms[i] = J2DAnmLoaderDataBase::Load(name.values(), m_archive);
        m_mainScreens[i].setAnimation(m_mainAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_mainAnmTextureSRTKeys.count(); i++) {
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "select_character%u.btk", i + 1);
        m_mainAnmTextureSRTKeys[i] = J2DAnmLoaderDataBase::Load(name.values(), m_archive);
        m_mainAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_mainScreens[i]);
        m_mainScreens[i].setAnimation(m_mainAnmTextureSRTKeys[i]);
    }
    for (u32 i = 0; i < m_mainAnmTevRegKeys.count(); i++) {
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "select_character%u.brk", i + 2);
        m_mainAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load(name.values(), m_archive);
        m_mainAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreens[i + 1]);
        m_mainScreens[i + 1].setAnimation(m_mainAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_mainAnmColors.count(); i++) {
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "select_character%u.bpk", i + 1);
        m_mainAnmColors[i] = J2DAnmLoaderDataBase::Load(name.values(), m_archive);
        m_mainAnmColors[i]->searchUpdateMaterialID(&m_mainScreens[i]);
        m_mainScreens[i].setAnimation(m_mainAnmColors[i]);
    }
    for (u32 i = 0; i < m_arrowAnmTransforms.count(); i++) {
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "select_character%u.bck", i + 1);
        m_arrowAnmTransforms[i] = J2DAnmLoaderDataBase::Load(name.values(), m_archive);
        for (u32 j = 1; j <= i + 1; j++) {
            m_mainScreens[i].search("ArL%u%u", i + 1, j)->setAnimation(m_arrowAnmTransforms[i]);
            m_mainScreens[i].search("ArR%u%u", i + 1, j)->setAnimation(m_arrowAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_colAnmTransforms.count(); i++) {
        m_colAnmTransforms[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bck", m_archive);
        m_colScreens[i / 2].search("Cha%u", i % 2 + 1)->setAnimation(m_colAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_colAnmTextureSRTKeys.count(); i++) {
        m_colAnmTextureSRTKeys[i] =
                J2DAnmLoaderDataBase::Load("select_character_cc.btk", m_archive);
        m_colAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_colScreens[i]);
        m_colScreens[i].setAnimation(m_colAnmTextureSRTKeys[i]);
    }
    for (u32 i = 0; i < m_windowAnmTevRegKeys.count(); i++) {
        m_windowAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("select_character_cc.brk", m_archive);
        m_windowAnmTevRegKeys[i]->searchUpdateMaterialID(&m_colScreens[i / 2]);
        m_colScreens[i / 2].search("CW%u", i % 2 + 1)->setAnimation(m_windowAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_outlineAnmTevRegKeys.count(); i++) {
        m_outlineAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("select_character_cc.brk", m_archive);
        m_outlineAnmTevRegKeys[i]->searchUpdateMaterialID(&m_colScreens[i / 2]);
        m_colScreens[i / 2].search("CWC%u", i % 2 + 1)->setAnimation(m_outlineAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_padAnmColors.count(); i++) {
        m_padAnmColors[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bpk", m_archive);
        m_padAnmColors[i]->searchUpdateMaterialID(&m_colScreens[i / 2]);
        m_colScreens[i / 2].search("CC%u", i % 2 + 1)->setAnimation(m_padAnmColors[i]);
    }
    for (u32 i = 0; i < m_characterAnmColors.count(); i++) {
        m_characterAnmColors[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bpk", m_archive);
        m_characterAnmColors[i]->searchUpdateMaterialID(&m_colScreens[i / 2]);
        m_colScreens[i / 2].search("CP%u", i % 2 + 1)->setAnimation(m_characterAnmColors[i]);
    }
    for (u32 i = 0; i < m_windowAnmColors.count(); i++) {
        m_windowAnmColors[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bpk", m_archive);
        m_windowAnmColors[i]->searchUpdateMaterialID(&m_colScreens[i / 2]);
        m_colScreens[i / 2].search("CW%u", i % 2 + 1)->setAnimation(m_windowAnmColors[i]);
    }
    for (u32 i = 0; i < m_extraAnmTransforms.count(); i++) {
        m_extraAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("select_character_plus1.bck", m_archive);
        m_extraScreen.search("N11%c", "FB"[i])->setAnimation(m_extraAnmTransforms[i]);
        if (i == 0) {
            m_extraScreen.search("N11H")->setAnimation(m_extraAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_extraAnmTevRegKeys.count(); i++) {
        m_extraAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("select_character_plus1.brk", m_archive);
        m_extraAnmTevRegKeys[i]->searchUpdateMaterialID(&m_extraScreen);
        m_extraScreen.search("N11%c", "FB"[i])->setAnimation(m_extraAnmTevRegKeys[i]);
    }
    m_extraAnmColor = J2DAnmLoaderDataBase::Load("select_character_plus1.bpk", m_archive);
    m_extraAnmColor->searchUpdateMaterialID(&m_extraScreen);
    m_extraScreen.setAnimation(m_extraAnmColor);

    m_mainAnmTextureSRTKeyFrame = 0;
    m_mainAnmColorFrame = 0;
    m_arrowAnmTransformFrame = 0;
    m_colAnmTextureSRTKeyFrames.fill(0);
}

SceneCharacterSelect::~SceneCharacterSelect() {}

void SceneCharacterSelect::init() {
    m_padCount = SequenceInfo::Instance().m_padCount;
    m_statusCount = RaceInfo::Instance().m_statusCount;
    J2DScreen &mainScreen = m_mainScreens[m_statusCount - 1];
    for (u32 i = 0; i < Min<u32>(m_statusCount * 2, 4); i++) {
        u32 j = i % m_statusCount;
        u32 k = i / m_statusCount;
        J2DPicture *picture =
                mainScreen.search("C%u%uP%u", m_statusCount, j + 1, k + 1)->downcast<J2DPicture>();
        picture->m_isVisible = i < m_padCount;
        if (i < m_padCount) {
            u32 l = Min(j * 2 + k, i + m_padCount - m_statusCount);
            m_statuses[l] = j;
            if (k == 0) {
                m_pads[j][0] = l;
            }
            m_pads[j][1] = i;
            Array<char, 32> name;
            snprintf(name.values(), name.count(), "SC_P%u_u.bti", l + 1);
            picture->changeTexture(name.values(), 0);
        }
    }
    m_characterIndices.fill(CharacterID::Count);
    OnlineInfo &onlineInfo = OnlineInfo::Instance();
    if (onlineInfo.m_hasIDs) {
        m_characterIDs = onlineInfo.m_characterIDs;
        m_kartIDs = onlineInfo.m_kartIDs;
        for (u32 i = 0; i < m_statusCount; i++) {
            deselectKart(i);
        }
    } else {
        switch (m_padCount) {
        case 1:
            m_characterIndices[0] = 0;
            break;
        case 2:
            m_characterIndices[0] = 0;
            m_characterIndices[1] = 16;
            break;
        case 3:
            m_characterIndices[0] = 0;
            m_characterIndices[1] = 2;
            m_characterIndices[2] = 16;
            break;
        case 4:
            m_characterIndices[0] = 0;
            m_characterIndices[1] = 2;
            m_characterIndices[2] = 14;
            m_characterIndices[3] = 16;
            break;
        }
        m_kartIndices.fill(KartID::Count);
        m_characterIDs.fill(CharacterID::Count);
        m_kartIDs.fill(KartID::Count);
    }
    m_spinFrame = 0;

    slideIn();
}

void SceneCharacterSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreens[m_statusCount - 1].draw(0.0f, 0.0f, m_graphContext);
    for (u32 i = 0; i < m_colScreens.count(); i++) {
        f32 x = m_colBox.start.x + (m_colBox.end.x - m_colBox.start.x) * (i * 2 + 1) / 20.0f;
        f32 y = m_colBox.start.y + (m_colBox.end.y - m_colBox.start.y) * (i * 2 + 1) / 20.0f;
        m_colScreens[i].draw(x, y, m_graphContext);
    }

    CharacterSelect3D *characterSelect3D = CharacterSelect3D::Instance();
    if (characterSelect3D) {
        f32 w, h = 348.0f;
        switch (m_statusCount) {
        case 1:
            w = 608.0f;
            break;
        case 2:
            w = 480.0f;
            break;
        case 3:
            w = 320.0f;
            break;
        case 4:
            w = 230.0f;
            break;
        }
        for (u32 i = 0; i < m_statusCount; i++) {
            J2DScreen &mainScreen = m_mainScreens[m_statusCount - 1];
            J2DPane *pane = mainScreen.search("HC%u%uw", m_statusCount, i + 1);
            const TBox<f32> &globalBox = pane->getGlobalBox();
            f32 x = (globalBox.start.x + globalBox.end.x - w) * 0.5f;
            if (x < 0.0f) {
                s32 xOffset = AlignUp(-floor(x) * J2DPane::GetARScale(), 2);
                GXSetScissorBoxOffset(xOffset, 0);
                x += xOffset / J2DPane::GetARScale();
            } else {
                GXSetScissorBoxOffset(0, 0);
            }
            J2DPane *parentPane = mainScreen.search("N_HC%u", m_statusCount);
            f32 y = 100.0f + parentPane->m_offset.y - 325.0f;
            J2DOrthoGraph orthoGraph(x, y, w, h, -1.0f, 1.0f);
            orthoGraph.setViewport();
            characterSelect3D->draw(i, w / h);
        }
    }

    m_graphContext->setViewport();

    if (m_statusCount == 1) {
        m_extraScreen.draw(0.0f, 0.0f, m_graphContext);
    }
}

void SceneCharacterSelect::calc() {
    (this->*m_state)();

    Array<Vec3f, 2> characterTranslations;
    Array<Vec3f, 2> characterRotations;
    Array<f32, 2> characterScales;
    Vec3f kartTranslation;
    Vec3f kartRotation;
    f32 kartScale;
    switch (m_statusCount) {
    case 1:
        characterTranslations[0] = Vec3f(170.0f, 110.0f, 380.0f);
        characterRotations[0] = Vec3f(352.0f, 346.0f, 4.0f);
        characterScales[0] = 0.95f;
        characterTranslations[1] = Vec3f(-385.0f, -150.0f, -380.0f);
        characterRotations[1] = Vec3f(353.0f, 13.6f, 356.0f);
        characterScales[1] = 1.85f;
        kartTranslation = Vec3f(0.0f, 5.0f, 0.0f);
        kartRotation = Vec3f(0.0f, 320.0f, 0.0f);
        kartScale = 1.2f;
        break;
    case 2:
        characterTranslations[0] = Vec3f(110.0f, 58.0f, 265.0f);
        characterRotations[0] = Vec3f(353.0f, 353.0f, 2.0f);
        characterScales[0] = 1.0f;
        characterTranslations[1] = Vec3f(-190.0f, -25.0f, -380.0f);
        characterRotations[1] = Vec3f(355.0f, 7.5f, 0.0f);
        characterScales[1] = 1.4f;
        kartTranslation = Vec3f(-10.0f, 0.0f, 0.0f);
        kartRotation = Vec3f(0.0f, 320.0f, 0.0f);
        kartScale = 1.13f;
        break;
    case 3:
        characterTranslations[0] = Vec3f(64.0f, 62.0f, 265.0f);
        characterRotations[0] = Vec3f(354.0f, 354.0f, 0.5f);
        characterScales[0] = 0.85f;
        characterTranslations[1] = Vec3f(-90.0f, -15.0f, -380.0f);
        characterRotations[1] = Vec3f(357.0f, 2.5f, 0.0f);
        characterScales[1] = 1.3f;
        kartTranslation = Vec3f(-10.0f, 10.0f, 0.0f);
        kartRotation = Vec3f(0.0f, 320.0f, 0.0f);
        kartScale = 1.0f;
        break;
    case 4:
        characterTranslations[0] = Vec3f(45.0f, 60.0f, 265.0f);
        characterRotations[0] = Vec3f(351.0f, 353.5f, 1.0f);
        characterScales[0] = 0.75f;
        characterTranslations[1] = Vec3f(-65.0f, 5.0f, -380.0f);
        characterRotations[1] = Vec3f(356.0f, 2.4f, 0.0f);
        characterScales[1] = 1.15f;
        kartTranslation = Vec3f(-10.0f, 15.0f, 0.0f);
        kartRotation = Vec3f(0.0f, 320.0f, 0.0f);
        kartScale = 0.85f;
        break;
    }
    CharacterSelect3D *characterSelect3D = CharacterSelect3D::Instance();
    if (characterSelect3D) {
        for (u32 i = 0; i < m_statusCount; i++) {
            u32 status = 1;
            u32 kartID = m_kartIDs[i];
            if (kartID == KartID::Count) {
                u32 kartIndex = m_kartIndices[i];
                if (kartIndex != KartID::Count) {
                    kartID = KartIDs[kartIndex];
                }
            } else {
                status = 2;
            }
            if (kartID != KartID::Count) {
                characterSelect3D->setKart(i, kartID, kartTranslation, kartRotation, kartScale);
            }
            for (u32 j = 0; j < 2; j++) {
                u32 characterID = m_characterIDs[i][j];
                if (characterID == CharacterID::Count || m_spinFrame != 0) {
                    characterSelect3D->setCharacterStatus(i, j, 0);
                } else {
                    characterSelect3D->setCharacterStatus(i, j, status);
                    characterSelect3D->setCharacter(i, j, characterID + 1, characterTranslations[j],
                            characterRotations[j], characterScales[j]);
                }
            }
        }
    }

    J2DPane *randomPane = m_mainScreens[m_statusCount - 1].search("NRandom");
    randomPane->m_isVisible = true;
    for (u32 i = 0; i < m_statusCount * 2; i++) {
        if (m_characterIDs[i / 2][i % 2] != CharacterID::Count) {
            randomPane->m_isVisible = false;
            break;
        }
    }
    for (u32 i = 0; i < m_statusCount; i++) {
        J2DPane *pane;
        if (m_statusCount == 1) {
            pane = m_mainScreens[m_statusCount - 1].search("N_P1");
        } else {
            pane = m_mainScreens[m_statusCount - 1].search("N_P%u%u", m_statusCount, i + 1);
        }
        pane->m_isVisible = m_kartIndices[i] != KartID::Count;
    }
    for (u32 i = 0; i < CharacterID::Count; i++) {
        J2DPicture *picture = m_colScreens[i / 2].search("CC%u", i % 2 + 1)->downcast<J2DPicture>();
        for (u32 j = 0; j < m_padCount; j++) {
            if (m_characterIndices[j] == i) {
                Array<char, 32> name;
                snprintf(name.values(), name.count(), "SC_P%u_%c.bti", j + 1, "us"[i % 2]);
                picture->changeTexture(name.values(), 0);
            }
        }
    }
    if (m_statusCount == 1) {
        for (u32 i = 0; i < m_characterIDs[0].count(); i++) {
            u32 characterID = m_characterIDs[0][i];
            if (characterID == CharacterID::Count) {
                continue;
            }
            J2DPicture *picture = m_extraScreen.search("N11%c", "FB"[i])->downcast<J2DPicture>();
            picture->changeTexture(CharacterNames[GetCharacterIndex(characterID)], 0);
        }
        u32 kartIndex = m_kartIndices[0];
        m_extraScreen.search("N_MP11")->m_isVisible = kartIndex != KartID::Count;
        if (kartIndex != KartID::Count) {
            for (u32 i = 0; i < KartStars[kartIndex].count(); i++) {
                for (u32 j = 0; j < 5; j++) {
                    J2DPane *pane = m_extraScreen.search("MP11%c%u", "SAW"[i], j + 1);
                    pane->m_isVisible = KartStars[kartIndex][i] > j;
                }
            }
        }
    }

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();
    if (characterSelect3D) {
        characterSelect3D->calc();
    }

    m_mainAnmTextureSRTKeyFrame = (m_mainAnmTextureSRTKeyFrame + 1) % 120;
    m_mainAnmColorFrame = (m_mainAnmColorFrame + 1) % 120;
    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 29;
    for (u32 i = 0; i < m_colAnmTextureSRTKeyFrames.count(); i++) {
        m_colAnmTextureSRTKeyFrames[i] = (m_colAnmTextureSRTKeyFrames[i] + 1) % 120;
    }
    for (u32 i = 0; i < CharacterID::Count; i++) {
        m_windowAnmTevRegKeyFrames[i] = 0;
        m_outlineAnmTevRegKeyFrames[i] = 0;
        m_padAnmColorFrames[i] = 0;
        m_characterAnmColorFrames[i] = 1;
        m_windowAnmColorFrames[i] = 0;
        if (m_colAnmTransformFrames[i] > 18) {
            for (u32 j = 0; j < m_padCount; j++) {
                if (m_characterIndices[j] == i) {
                    m_outlineAnmTevRegKeyFrames[i] = j + 1;
                    m_padAnmColorFrames[i] = j + 1;
                }
            }
        }
        for (u32 j = 0; j < m_statusCount; j++) {
            for (u32 k = 0; k < m_characterIDs[j].count(); k++) {
                if (m_characterIDs[j][k] == CharacterIDs[i]) {
                    u32 pad = m_pads[j][k];
                    if ((m_colAnmTransformFrames[i] <= 18 || m_colAnmTransformFrames[i] == 26) &&
                            m_spinFrame == 0) {
                        m_windowAnmTevRegKeyFrames[i] = pad + 1;
                        m_characterAnmColorFrames[i] = 0;
                        m_windowAnmColorFrames[i] = pad + 1;
                    } else if (m_characterIndices[pad] == CharacterID::Count) {
                        m_outlineAnmTevRegKeyFrames[i] = pad + 1;
                        m_padAnmColorFrames[i] = pad + 1;
                    }
                }
            }
        }
    }
    if (m_statusCount == 1) {
        for (u32 i = 0; i < m_characterIDs[0].count(); i++) {
            u32 characterID = m_characterIDs[0][i];
            if (characterID != CharacterID::Count &&
                    m_colAnmTransformFrames[GetCharacterIndex(characterID)] > 18 &&
                    m_spinFrame == 0 && m_kartIDs[0] == KartID::Count) {
                if (m_extraAnmTransformFrames[i] < 10) {
                    m_extraAnmTransformFrames[i]++;
                }
            } else {
                m_extraAnmTransformFrames[i] = 0;
            }
            m_extraAnmTevRegKeyFrames[i] = m_extraAnmTransformFrames[i];
            if (i == 0) {
                m_extraAnmColorFrame = m_extraAnmTransformFrames[i];
            }
        }
    }

    m_mainAnmTransforms[m_statusCount - 1]->m_frame = m_mainAnmTransformFrame;
    m_mainAnmTextureSRTKeys[m_statusCount - 1]->m_frame = m_mainAnmTextureSRTKeyFrame;
    if (m_statusCount == 2 || m_statusCount == 3) {
        m_mainAnmTevRegKeys[m_statusCount - 2]->m_frame = m_mainAnmTevRegKeyFrame;
    }
    m_mainAnmColors[m_statusCount - 1]->m_frame = m_mainAnmColorFrame;
    m_arrowAnmTransforms[m_statusCount - 1]->m_frame = m_arrowAnmTransformFrame;
    for (u32 i = 0; i < m_colAnmTransforms.count(); i++) {
        m_colAnmTransforms[i]->m_frame = m_colAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_colAnmTextureSRTKeys.count(); i++) {
        m_colAnmTextureSRTKeys[i]->m_frame = m_colAnmTextureSRTKeyFrames[i];
    }
    for (u32 i = 0; i < m_windowAnmTevRegKeys.count(); i++) {
        m_windowAnmTevRegKeys[i]->m_frame = m_windowAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_outlineAnmTevRegKeys.count(); i++) {
        m_outlineAnmTevRegKeys[i]->m_frame = m_outlineAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_padAnmColors.count(); i++) {
        m_padAnmColors[i]->m_frame = m_padAnmColorFrames[i];
    }
    for (u32 i = 0; i < m_characterAnmColors.count(); i++) {
        m_characterAnmColors[i]->m_frame = m_characterAnmColorFrames[i];
    }
    for (u32 i = 0; i < m_windowAnmColors.count(); i++) {
        m_windowAnmColors[i]->m_frame = m_windowAnmColorFrames[i];
    }
    if (m_statusCount == 1) {
        for (u32 i = 0; i < m_extraAnmTransforms.count(); i++) {
            m_extraAnmTransforms[i]->m_frame = m_extraAnmTransformFrames[i];
        }
        for (u32 i = 0; i < m_extraAnmTevRegKeys.count(); i++) {
            m_extraAnmTevRegKeys[i]->m_frame = m_extraAnmTevRegKeyFrames[i];
        }
        m_extraAnmColor->m_frame = m_extraAnmColorFrame;
    }

    m_mainScreens[m_statusCount - 1].animation();
    for (u32 i = 0; i < m_colScreens.count(); i++) {
        m_colScreens[i].animation();
    }
    if (m_statusCount == 1) {
        m_extraScreen.animation();
    }
}

void SceneCharacterSelect::slideIn() {
    m_heap = JKRExpHeap::Create(640 * 1024, m_parentHeap, false);
    m_heap->becomeCurrentHeap();
    CharacterSelect3D::Create(m_heap);
    m_parentHeap->becomeCurrentHeap();
    CharacterSelect3D::Instance()->init();

    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::SelectCharacter);
    m_mainAnmTransformFrame = 0;
    if (m_statusCount == 2 || m_statusCount == 3) {
        m_mainAnmTevRegKeyFrame = m_padCount != m_statusCount;
    }
    m_colAnmTransformFrames.fill(0);
    if (m_statusCount == 1) {
        m_extraAnmTransformFrames.fill(0);
        m_extraAnmTevRegKeyFrames.fill(0);
        m_extraAnmColorFrame = 0;
    }
    if (m_statusCount != 1) {
        J2DScreen &mainScreen = m_mainScreens[m_statusCount - 1];
        for (u32 i = 0; i < m_statusCount; i++) {
            J2DPane *pane = mainScreen.search("Wait%u%u", m_statusCount, i + 1);
            pane->m_isVisible = false;
        }
    }
    m_state = &SceneCharacterSelect::stateSlideIn;
}

void SceneCharacterSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    if (m_statusCount != 1) {
        J2DScreen &mainScreen = m_mainScreens[m_statusCount - 1];
        for (u32 i = 0; i < m_statusCount; i++) {
            J2DPane *pane = mainScreen.search("Wait%u%u", m_statusCount, i + 1);
            pane->m_isVisible = false;
        }
    }
    m_state = &SceneCharacterSelect::stateSlideOut;
}

void SceneCharacterSelect::idle() {
    m_state = &SceneCharacterSelect::stateIdle;
    m_spinFrame = 0;
}

void SceneCharacterSelect::spin() {
    m_spinFrame = 1;
    m_state = &SceneCharacterSelect::stateSpin;
}

void SceneCharacterSelect::nextScene() {
    CharacterSelect3D::Destroy();
    m_heap->destroy();
    m_state = &SceneCharacterSelect::stateNextScene;
}

void SceneCharacterSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 19) {
        m_mainAnmTransformFrame++;
        m_colAnmTransformFrames.fill(Min<u32>(m_mainAnmTransformFrame, 18));
    } else {
        m_colAnmTransformFrames.fill(26);
        idle();
    }
}

void SceneCharacterSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_colAnmTransformFrames.fill(Min<u32>(m_mainAnmTransformFrame, 18));
    } else {
        nextScene();
    }
}

void SceneCharacterSelect::stateIdle() {
    for (u32 i = 0; i < m_padCount; i++) {
        u32 selectedKartCount = countSelectedKarts();
        if (selectedKartCount == m_statusCount) {
            break;
        }

        u32 statusIndex = m_statuses[i];
        const JUTGamePad::CButton &button = KartGamePad::GamePad(i)->button();
        if (button.risingEdge() & PAD_BUTTON_A) {
            selectCharacter(i);
            selectKart(statusIndex);
        } else if (button.risingEdge() & PAD_BUTTON_B) {
            if (i == 0) {
                u32 selectedCharacterCount = 0;
                for (u32 j = 0; j < m_pads[statusIndex].count(); j++) {
                    u32 pad = m_pads[statusIndex][j];
                    u32 characterID = m_characterIDs[statusIndex][j];
                    selectedCharacterCount += pad == i && characterID != CharacterID::Count;
                }
                if (selectedCharacterCount == 0) {
                    m_nextScene = SceneType::FormatSelect;
                    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
                    slideOut();
                    break;
                }
            }
            deselectCharacter(i);
            deselectKart(statusIndex);
        } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
            if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
                move(i, -1, -1);
            } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
                move(i, 1, -1);
            } else {
                move(i, 0, -1);
            }
        } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
            if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
                move(i, -1, 1);
            } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
                move(i, 1, 1);
            } else {
                move(i, 0, 1);
            }
        } else if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
            move(i, -1, 0);
            changeKart(statusIndex, -1);
        } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
            move(i, 1, 0);
            changeKart(statusIndex, 1);
        } else if (button.risingEdge() & PAD_TRIGGER_Z) {
            if (m_kartIndices[statusIndex] != KartID::Count) {
                Swap(m_pads[statusIndex][0], m_pads[statusIndex][1]);
                Swap(m_characterIDs[statusIndex][0], m_characterIDs[statusIndex][1]);
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            }
        } else if ((button.level() & PAD_TRIGGER_R) && (button.level() & PAD_TRIGGER_L)) {
            if (i == 0 && countSelectedCharacters() == 0) {
                spin();
                break;
            }
        }
    }

    for (u32 i = 0; i < m_colAnmTransformFrames.count(); i++) {
        if (m_colAnmTransformFrames[i] == 26) {
            continue;
        }

        m_colAnmTransformFrames[i]++;
        if (m_colAnmTransformFrames[i] != 26) {
            continue;
        }

        for (u32 j = 0; j < m_padCount; j++) {
            if (m_characterIndices[j] == i) {
                u32 statusIndex = m_statuses[j];
                if (m_pads[statusIndex][1] == j &&
                        m_characterIDs[statusIndex][1] == CharacterID::Count) {
                    u32 characterIndex = i;
                    do {
                        characterIndex += CharacterID::Count + 1;
                    } while (!move(j, characterIndex));
                } else {
                    m_characterIndices[j] = CharacterID::Count;
                }
                break;
            }
        }
    }

    for (u32 i = 0; i < m_statusCount; i++) {
        if (m_kartIndices[i] != KartID::Count || m_kartIDs[i] != KartID::Count) {
            continue;
        }
        u32 frontCharacterIndex = m_characterIndices[m_pads[i][0]];
        if (frontCharacterIndex != CharacterID::Count) {
            continue;
        }
        u32 backCharacterIndex = m_characterIndices[m_pads[i][1]];
        if (backCharacterIndex != CharacterID::Count) {
            continue;
        }
        u32 frontCharacterID = m_characterIDs[i][0];
        u32 backCharacterID = m_characterIDs[i][1];
        if (m_colAnmTransformFrames[GetCharacterIndex(frontCharacterID)] != 26) {
            continue;
        }
        if (m_colAnmTransformFrames[GetCharacterIndex(backCharacterID)] != 26) {
            continue;
        }
        u32 frontCharacterWeight = KartInfo::GetDriverWeight(frontCharacterID + 1);
        u32 backCharacterWeight = KartInfo::GetDriverWeight(backCharacterID + 1);
        u32 kartID = KartInfo::GetDefaultKartID(frontCharacterID + 1);
        if (frontCharacterWeight < backCharacterWeight) {
            kartID = KartInfo::GetDefaultKartID(backCharacterID + 1);
        }
        for (u32 j = 0; j < KartID::Count; j++) {
            if (KartIDs[j] == kartID) {
                m_kartIndices[i] = j;
                break;
            }
        }
    }

    if (m_statusCount != 1) {
        J2DScreen &mainScreen = m_mainScreens[m_statusCount - 1];
        for (u32 i = 0; i < m_statusCount; i++) {
            J2DPane *pane = mainScreen.search("Wait%u%u", m_statusCount, i + 1);
            pane->m_isVisible = m_kartIDs[i] != KartID::Count;
        }
    }

    u32 selectedKartCount = countSelectedKarts();
    if (selectedKartCount != m_statusCount) {
        return;
    }

    for (u32 i = 0; i < m_statusCount; i++) {
        if (!CharacterSelect3D::Instance()->isNext(i)) {
            return;
        }
    }

    m_nextScene = SceneType::MapSelect;
    slideOut();
}

void SceneCharacterSelect::stateSpin() {
    m_spinFrame++;
    bool isSpinning = m_spinFrame <= 25;
    if (!isSpinning) {
        for (u32 i = 0; i < m_padCount; i++) {
            const JUTGamePad::CButton &button = KartGamePad::GamePad(i)->button();
            if ((button.level() & PAD_TRIGGER_R) && (button.level() & PAD_TRIGGER_L)) {
                isSpinning = true;
                break;
            }
        }
    }

    if (!isSpinning) {
        for (u32 i = 0; i < m_statusCount; i++) {
            for (u32 j = 0; j < 2; j++) {
                u32 characterIndex = GetCharacterIndex(m_characterIDs[i][j]);
                m_colAnmTransformFrames[characterIndex] = 19;
            }
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        idle();
        return;
    }

    if (isSpinning && m_spinFrame % 5 != 0) {
        return;
    }

    m_characterIndices.fill(CharacterID::Count);
    u32 flags = 0;
    for (u32 i = 0; i < m_statusCount; i++) {
        for (u32 j = 0; j < 2; j++) {
            u32 characterIndex = OSGetTime() % (CharacterID::Count - (i * 2 + j));
            for (u32 k = 0; k < CharacterID::Count && k <= characterIndex; k++) {
                characterIndex += flags >> k & 1;
            }
            flags |= 1 << characterIndex;
            m_characterIDs[i][j] = CharacterIDs[characterIndex];
        }
    }
    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
}

void SceneCharacterSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

u32 SceneCharacterSelect::countSelectedCharacters() const {
    u32 selectedCharacterCount = 0;
    for (u32 i = 0; i < m_statusCount; i++) {
        for (u32 j = 0; j < m_characterIDs[i].count(); j++) {
            selectedCharacterCount += m_characterIDs[i][j] != CharacterID::Count;
        }
    }
    return selectedCharacterCount;
}

u32 SceneCharacterSelect::countSelectedKarts() const {
    u32 selectedKartCount = 0;
    for (u32 i = 0; i < m_statusCount; i++) {
        selectedKartCount += m_kartIDs[i] != KartID::Count;
    }
    return selectedKartCount;
}

void SceneCharacterSelect::move(u32 padIndex, s32 x, s32 y) {
    u32 characterIndex = m_characterIndices[padIndex];
    if (characterIndex == CharacterID::Count) {
        return;
    }

    if (x < 0) {
        if (y < 0) {
            if (characterIndex % 2 == 1) {
                move(padIndex, characterIndex + CharacterID::Count - 2 - 1);
            }
        } else if (y > 0) {
            if (characterIndex % 2 == 0) {
                move(padIndex, characterIndex + CharacterID::Count - 2 + 1);
            }
        } else {
            do {
                characterIndex += CharacterID::Count - 2;
            } while (!move(padIndex, characterIndex));
        }
    } else if (x > 0) {
        if (y < 0) {
            if (characterIndex % 2 == 1) {
                move(padIndex, characterIndex + CharacterID::Count + 2 - 1);
            }
        } else if (y > 0) {
            if (characterIndex % 2 == 0) {
                move(padIndex, characterIndex + CharacterID::Count + 2 + 1);
            }
        } else {
            do {
                characterIndex += CharacterID::Count + 2;
            } while (!move(padIndex, characterIndex));
        }
    } else {
        move(padIndex, characterIndex ^ 1);
    }
}

bool SceneCharacterSelect::move(u32 padIndex, u32 characterIndex) {
    if (m_colAnmTransformFrames[m_characterIndices[padIndex]] != 26) {
        return true;
    }

    characterIndex %= CharacterID::Count;
    for (u32 i = 0; i < m_padCount; i++) {
        if (m_characterIndices[i] == characterIndex) {
            return false;
        }
    }
    for (u32 i = 0; i < m_statusCount; i++) {
        for (u32 j = 0; j < m_characterIDs[i].count(); j++) {
            if (m_characterIDs[i][j] == CharacterIDs[characterIndex]) {
                return false;
            }
        }
    }

    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    m_characterIndices[padIndex] = characterIndex;
    return true;
}

void SceneCharacterSelect::selectCharacter(u32 padIndex) {
    u32 characterIndex = m_characterIndices[padIndex];
    if (characterIndex == CharacterID::Count) {
        return;
    }

    if (m_colAnmTransformFrames[characterIndex] != 26) {
        return;
    }

    u32 statusIndex = m_statuses[padIndex];
    for (u32 i = 0; i < 2; i++) {
        if (m_pads[statusIndex][i] == padIndex &&
                m_characterIDs[statusIndex][i] == CharacterID::Count) {
            m_characterIDs[statusIndex][i] = CharacterIDs[characterIndex];
            m_colAnmTransformFrames[characterIndex] = 19;
            GameAudio::Main::Instance()->startSystemSe(CharacterSelectionSoundIDs[characterIndex]);
            break;
        }
    }
}

void SceneCharacterSelect::deselectCharacter(u32 padIndex) {
    u32 statusIndex = m_statuses[padIndex];
    if (m_kartIDs[statusIndex] != KartID::Count) {
        return;
    }

    m_kartIndices[statusIndex] = KartID::Count;
    for (u32 i = 2; i-- > 0;) {
        if (m_pads[statusIndex][i] != padIndex) {
            continue;
        }
        u32 characterID = m_characterIDs[statusIndex][i];
        if (characterID != CharacterID::Count) {
            for (u32 j = 0; j < CharacterID::Count; j++) {
                if (CharacterIDs[j] == characterID) {
                    m_characterIndices[padIndex] = j;
                    m_characterIDs[statusIndex][i] = CharacterID::Count;
                    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE2);
                    return;
                }
            }
        }
    }
}

void SceneCharacterSelect::changeKart(u32 statusIndex, s32 x) {
    u32 kartIndex = m_kartIndices[statusIndex];
    if (kartIndex == KartID::Count) {
        return;
    }

    u32 frontCharacterID = m_characterIDs[statusIndex][0];
    u32 backCharacterID = m_characterIDs[statusIndex][1];
    u32 frontCharacterWeight = KartInfo::GetDriverWeight(frontCharacterID + 1);
    u32 backCharacterWeight = KartInfo::GetDriverWeight(backCharacterID + 1);
    u32 weight = Max(frontCharacterWeight, backCharacterWeight);
    u32 kartID;
    do {
        kartIndex = (kartIndex + KartID::Count + x) % KartID::Count;
        kartID = KartIDs[kartIndex];
    } while (kartID != KartID::Extra && KartInfo::GetKartWeight(kartID) != weight);
    m_kartIndices[statusIndex] = kartIndex;
    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
}

void SceneCharacterSelect::selectKart(u32 statusIndex) {
    u32 kartIndex = m_kartIndices[statusIndex];
    if (kartIndex == KartID::Count) {
        return;
    }

    m_kartIDs[statusIndex] = KartIDs[kartIndex];
    m_kartIndices[statusIndex] = KartID::Count;
    for (u32 i = 0; i < m_characterIDs[statusIndex].count(); i++) {
        for (u32 j = 0; j < CharacterID::Count; j++) {
            if (CharacterIDs[j] == m_characterIDs[statusIndex][i]) {
                GameAudio::Main::Instance()->startSystemSe(KartSelectionSoundIDs[j]);
                break;
            }
        }
    }
}

void SceneCharacterSelect::deselectKart(u32 statusIndex) {
    for (u32 i = 0; i < KartID::Count; i++) {
        if (KartIDs[i] == m_kartIDs[statusIndex]) {
            if (CharacterSelect3D::Instance()->isCancel(statusIndex)) {
                m_kartIndices[statusIndex] = i;
                m_kartIDs[statusIndex] = KartID::Count;
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
            }
            break;
        }
    }
}

u32 SceneCharacterSelect::GetCharacterIndex(u32 characterID) {
    u32 characterIndex;
    for (characterIndex = 0; characterIndex < CharacterID::Count; characterIndex++) {
        if (CharacterIDs[characterIndex] == characterID) {
            break;
        }
    }
    return characterIndex;
}

const Array<u32, CharacterID::Count> SceneCharacterSelect::CharacterIDs((u32[CharacterID::Count]){
        CharacterID::Mario,
        CharacterID::Luigi,
        CharacterID::Peach,
        CharacterID::Daisy,
        CharacterID::Yoshi,
        CharacterID::Catherine,
        CharacterID::BabyMario,
        CharacterID::BabyLuigi,
        CharacterID::Kinopio,
        CharacterID::Kinopico,
        CharacterID::Nokonoko,
        CharacterID::Patapata,
        CharacterID::Donkey,
        CharacterID::Diddy,
        CharacterID::Koopa,
        CharacterID::KoopaJr,
        CharacterID::Wario,
        CharacterID::Waluigi,
        CharacterID::Pakkun,
        CharacterID::Teresa,
});
