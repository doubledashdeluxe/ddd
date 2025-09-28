#include "SceneProfileSelect.hh"

#include "game/ExtendedSystemRecord.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/SystemRecord.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

extern "C" {
#include <stdio.h>
}

SceneProfileSelect::SceneProfileSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);

    m_mainScreen.set("SelectProfile.blo", 0x1040000, lanEntryArchive);
    for (u32 i = 0; i < m_profileScreens.count(); i++) {
        m_profileScreens[i].set("SelectProfileProfile.blo", 0x1040000, lanEntryArchive);
    }

    for (u32 i = 0; i < m_profileScreens.count(); i++) {
        m_mainScreen.search("P%u", i)->appendChild(&m_profileScreens[i]);
    }

    for (u32 i = 0; i < m_profileAnmTransforms.count(); i++) {
        m_profileAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("SelectProfileProfile.bck", lanEntryArchive);
        m_profileScreens[i].search("Cha1")->setAnimation(m_profileAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_profileAnmTextureSRTKeys.count(); i++) {
        m_profileAnmTextureSRTKeys[i] =
                J2DAnmLoaderDataBase::Load("select_character_cc.btk", m_archive);
        m_profileAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_profileScreens[i]);
        m_profileScreens[i].setAnimation(m_profileAnmTextureSRTKeys[i]);
    }
    for (u32 i = 0; i < m_windowAnmTevRegKeys.count(); i++) {
        m_windowAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("select_character_cc.brk", m_archive);
        m_windowAnmTevRegKeys[i]->searchUpdateMaterialID(&m_profileScreens[i]);
        m_profileScreens[i].search("CW1")->setAnimation(m_windowAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_outlineAnmTevRegKeys.count(); i++) {
        m_outlineAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("select_character_cc.brk", m_archive);
        m_outlineAnmTevRegKeys[i]->searchUpdateMaterialID(&m_profileScreens[i]);
        m_profileScreens[i].search("CWC1")->setAnimation(m_outlineAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_padAnmColors.count(); i++) {
        m_padAnmColors[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bpk", m_archive);
        m_padAnmColors[i]->searchUpdateMaterialID(&m_profileScreens[i]);
        m_profileScreens[i].search("CC1")->setAnimation(m_padAnmColors[i]);
    }
    for (u32 i = 0; i < m_nameAnmColors.count(); i++) {
        m_nameAnmColors[i] =
                J2DAnmLoaderDataBase::Load("SelectProfileProfile.bpk", lanEntryArchive);
        m_nameAnmColors[i]->searchUpdateMaterialID(&m_profileScreens[i]);
        for (u32 j = 0; j < 3; j++) {
            m_profileScreens[i].search("Name%u", j)->setAnimation(m_nameAnmColors[i]);
        }
    }
    for (u32 i = 0; i < m_windowAnmColors.count(); i++) {
        m_windowAnmColors[i] = J2DAnmLoaderDataBase::Load("select_character_cc.bpk", m_archive);
        m_windowAnmColors[i]->searchUpdateMaterialID(&m_profileScreens[i]);
        m_profileScreens[i].search("CW1")->setAnimation(m_windowAnmColors[i]);
    }

    m_profileAnmTextureSRTKeyFrames.fill(0);
}

SceneProfileSelect::~SceneProfileSelect() {}

void SceneProfileSelect::init() {
    Array<Array<char, 4>, ProfileCount> names;
    names[0] = SystemRecord::Instance().m_name;
    ExtendedSystemRecord &extendedSystemRecord = ExtendedSystemRecord::Instance();
    for (u32 i = 0; i < extendedSystemRecord.m_names.count(); i++) {
        names[1 + i] = extendedSystemRecord.m_names[i];
    }
    for (u32 i = 0; i < names.count(); i++) {
        names[i][names[i].count() - 1] = '\0';
    }
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < m_profileScreens.count(); i++) {
        kart2DCommon->changeUnicodeTexture(names[i].values(), 3, m_profileScreens[i], "Name");
    }

    if (SequenceApp::Instance()->prevScene() == SceneType::HowManyPlayers) {
        m_padCount = SequenceInfo::Instance().m_padCount;
        for (u32 i = 0; i < m_profileIndices.count(); i++) {
            m_profileIndices[i] = i < m_padCount ? i : ProfileCount;
        }

        SequenceApp::Instance()->ready(SceneType::NameSelect);
    }
    m_selectedFlags = 0;

    slideIn();
}

void SceneProfileSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneProfileSelect::calc() {
    (this->*m_state)();

    for (u32 i = 0; i < m_padCount; i++) {
        if (!(m_selectedFlags & 1 << i)) {
            u32 profileIndex = m_profileIndices[i];
            J2DPicture *picture =
                    m_profileScreens[profileIndex].search("CC1")->downcast<J2DPicture>();
            Array<char, 32> name;
            snprintf(name.values(), name.count(), "SC_P%lu_u.bti", i + 1);
            picture->changeTexture(name.values(), 0);
        }
    }

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    for (u32 i = 0; i < m_profileAnmTextureSRTKeyFrames.count(); i++) {
        m_profileAnmTextureSRTKeyFrames[i] = (m_profileAnmTextureSRTKeyFrames[i] + 1) % 120;
    }
    for (u32 i = 0; i < ProfileCount; i++) {
        m_windowAnmTevRegKeyFrames[i] = 0;
        m_outlineAnmTevRegKeyFrames[i] = 0;
        m_padAnmColorFrames[i] = 0;
        m_nameAnmColorFrames[i] = 1;
        m_windowAnmColorFrames[i] = 0;
    }
    for (u32 i = 0; i < m_padCount; i++) {
        u32 profileIndex = m_profileIndices[i];
        if (m_selectedFlags & 1 << i) {
            m_windowAnmTevRegKeyFrames[profileIndex] = i + 1;
            m_nameAnmColorFrames[profileIndex] = 0;
            m_windowAnmColorFrames[profileIndex] = i + 1;
        } else {
            m_outlineAnmTevRegKeyFrames[profileIndex] = i + 1;
            m_padAnmColorFrames[profileIndex] = i + 1;
            m_windowAnmColorFrames[profileIndex] = 5;
        }
    }

    for (u32 i = 0; i < m_profileAnmTransforms.count(); i++) {
        m_profileAnmTransforms[i]->m_frame = m_profileAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_profileAnmTextureSRTKeys.count(); i++) {
        m_profileAnmTextureSRTKeys[i]->m_frame = m_profileAnmTextureSRTKeyFrames[i];
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
    for (u32 i = 0; i < m_nameAnmColors.count(); i++) {
        m_nameAnmColors[i]->m_frame = m_nameAnmColorFrames[i];
    }
    for (u32 i = 0; i < m_windowAnmColors.count(); i++) {
        m_windowAnmColors[i]->m_frame = m_windowAnmColorFrames[i];
    }

    m_mainScreen.animation();
    for (u32 i = 0; i < m_profileScreens.count(); i++) {
        m_profileScreens[i].animationMaterials();
    }
}

void SceneProfileSelect::slideIn() {
    MenuTitleLine::Instance()->drop("SelectProfile.bti");
    m_profileAnmTransformFrames.fill(0);
    m_state = &SceneProfileSelect::stateSlideIn;
}

void SceneProfileSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneProfileSelect::stateSlideOut;
}

void SceneProfileSelect::idle() {
    m_state = &SceneProfileSelect::stateIdle;
}

void SceneProfileSelect::nextScene() {
    m_state = &SceneProfileSelect::stateNextScene;
}

void SceneProfileSelect::stateSlideIn() {
    if (m_profileAnmTransformFrames[0] < 18) {
        m_profileAnmTransformFrames.fill(m_profileAnmTransformFrames[0] + 1);
    } else {
        idle();
    }
}

void SceneProfileSelect::stateSlideOut() {
    if (m_profileAnmTransformFrames[0] > 0) {
        m_profileAnmTransformFrames.fill(m_profileAnmTransformFrames[0] - 1);
    } else {
        nextScene();
    }
}

void SceneProfileSelect::stateIdle() {
    for (u32 i = 0; i < m_padCount; i++) {
        u32 selectedProfileCount = countSelectedProfiles();
        if (selectedProfileCount == m_padCount) {
            break;
        }

        const JUTGamePad::CButton &button = KartGamePad::GamePad(i)->button();
        if (button.risingEdge() & PAD_BUTTON_A) {
            selectProfile(i);
        } else if (button.risingEdge() & PAD_BUTTON_B) {
            if (i == 0) {
                if (!(m_selectedFlags & 1 << i)) {
                    m_nextScene = SceneType::HowManyPlayers;
                    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
                    slideOut();
                    break;
                }
            }
            deselectProfile(i);
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
        } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
            move(i, 1, 0);
        }
    }

    for (u32 i = 0; i < m_profileAnmTransformFrames.count(); i++) {
        if (m_profileAnmTransformFrames[i] > 18) {
            m_profileAnmTransformFrames[i]--;
        }
    }

    u32 selectedProfileCount = countSelectedProfiles();
    if (selectedProfileCount != m_padCount) {
        return;
    }

    for (u32 i = 0; i < m_profileAnmTransformFrames.count(); i++) {
        if (m_profileAnmTransformFrames[i] != 18) {
            return;
        }
    }

    m_nextScene = SceneType::NameSelect;
    OnlineInfo::Instance().m_profileIndices = m_profileIndices;
    slideOut();
}

void SceneProfileSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

u32 SceneProfileSelect::countSelectedProfiles() const {
    u32 selectedProfileCount = 0;
    for (u32 i = 0; i < m_padCount; i++) {
        selectedProfileCount += m_selectedFlags >> i & 1;
    }
    return selectedProfileCount;
}

void SceneProfileSelect::move(u32 padIndex, s32 x, s32 y) {
    if (m_selectedFlags & 1 << padIndex) {
        return;
    }

    u32 profileIndex = m_profileIndices[padIndex];
    u32 profileX = profileIndex % ColCount;
    u32 profileY = profileIndex / ColCount;
    if (x < 0) {
        if (y < 0) {
            if (profileX != 0 && profileY != 0) {
                move(padIndex, profileIndex + ProfileCount - 2 - 1);
            }
        } else if (y > 0) {
            if (profileX != 0 && profileY != RowCount - 1) {
                move(padIndex, profileIndex + ProfileCount - 2 + 1);
            }
        } else {
            do {
                profileIndex += ProfileCount - 1;
                if (profileX == 0) {
                    profileIndex += ColCount;
                }
                profileX = profileIndex % ColCount;
            } while (!move(padIndex, profileIndex));
        }
    } else if (x > 0) {
        if (y < 0) {
            if (profileX != ColCount - 1 && profileY != 0) {
                move(padIndex, profileIndex + ProfileCount - ColCount + 1);
            }
        } else if (y > 0) {
            if (profileX != ColCount - 1 && profileY != RowCount - 1) {
                move(padIndex, profileIndex + ProfileCount + ColCount + 1);
            }
        } else {
            do {
                profileIndex += ProfileCount + 1;
                if (profileX == ColCount - 1) {
                    profileIndex -= ColCount;
                }
                profileX = profileIndex % ColCount;
            } while (!move(padIndex, profileIndex));
        }
    } else if (y < 0) {
        do {
            profileIndex += ProfileCount - ColCount;
        } while (!move(padIndex, profileIndex));
    } else if (y > 0) {
        do {
            profileIndex += ProfileCount + ColCount;
        } while (!move(padIndex, profileIndex));
    }
}

bool SceneProfileSelect::move(u32 padIndex, u32 profileIndex) {
    profileIndex %= ProfileCount;
    for (u32 i = 0; i < m_padCount; i++) {
        if (m_profileIndices[i] == profileIndex) {
            return false;
        }
    }

    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    m_profileIndices[padIndex] = profileIndex;
    return true;
}

void SceneProfileSelect::selectProfile(u32 padIndex) {
    if (m_selectedFlags & 1 << padIndex) {
        return;
    }

    m_selectedFlags |= 1 << padIndex;
    m_profileAnmTransformFrames[m_profileIndices[padIndex]] = 25;
    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
}

void SceneProfileSelect::deselectProfile(u32 padIndex) {
    if (!(m_selectedFlags & 1 << padIndex)) {
        return;
    }

    if (m_profileAnmTransformFrames[m_profileIndices[padIndex]] != 18) {
        return;
    }

    m_selectedFlags &= ~(1 << padIndex);
    GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
}
