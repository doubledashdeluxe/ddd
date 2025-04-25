#include "SceneTandemSelect.hh"

#include "game/CharacterID.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/RaceInfo.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/Client.hh>
#include <payload/online/ServerManager.hh>

extern "C" {
#include <stdio.h>
}

SceneTandemSelect::SceneTandemSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_mainScreen.set("SingleTagSelect.blo", 0x1040000, m_archive);
    m_padCountScreen.set("PlayerIcon.blo", 0x1040000, m_archive);

    m_padCountScreen.search("Ns1234")->m_isVisible = false;
    m_padCountScreen.search("Ns12_3_4")->m_isVisible = false;
    m_padCountScreen.search("Ns12_34")->m_isVisible = false;

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < 3; i++) {
        for (u32 j = i == 2 ? 4 : 2; j <= 4; j++) {
            Array<u32, 4> characterIDs;
            characterIDs[0] = CharacterID::Mario;
            characterIDs[1] = i >= 1 && j == 4 ? CharacterID::Yoshi : CharacterID::Peach;
            characterIDs[2] = i >= 1 && j == 4 ? CharacterID::Peach : CharacterID::Yoshi;
            characterIDs[3] = CharacterID::Koopa;
            for (u32 k = 0; k < j; k++) {
                J2DPicture *picture =
                        m_mainScreen.search("C%ucha%u", j, i * j + k + 1)->downcast<J2DPicture>();
                picture->changeTexture(kart2DCommon->getCharacterIcon(characterIDs[k]), 0);
            }
        }
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SingleTagSelect.bck", m_archive);
    m_mainScreen.search("N_CoVs")->setAnimation(m_mainAnmTransform);
    m_mainAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("SingleTagSelect.btk", m_archive);
    m_mainAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmTextureSRTKey);
    m_mainAnmColor = J2DAnmLoaderDataBase::Load("SingleTagSelect.bpk", m_archive);
    m_mainAnmColor->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmColor);
    for (u32 i = 0; i < m_partitionAnmTransforms.count(); i++) {
        m_partitionAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SingleTagSelect.bck", m_archive);
        for (u32 j = i == 2 ? 4 : 2; j <= 4; j++) {
            m_mainScreen.search("C%ucrsr%u", j, i + 1)->setAnimation(m_partitionAnmTransforms[i]);
            m_mainScreen.search("CNP%uM%u", j, i + 1)->setAnimation(m_partitionAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_partitionAnmTextureSRTKeys.count(); i++) {
        m_partitionAnmTextureSRTKeys[i] =
                J2DAnmLoaderDataBase::Load("SingleTagSelect.btk", m_archive);
        m_partitionAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        for (u32 j = i == 2 ? 4 : 2; j <= 4; j++) {
            for (u32 k = GetFirstStatusIndex(j, i); k < GetFirstStatusIndex(j, i + 1); k++) {
                m_mainScreen.search("C%ucha_w%u", j, k)
                        ->setAnimation(m_partitionAnmTextureSRTKeys[i]);
            }
        }
    }
    for (u32 i = 0; i < m_partitionAnmTevRegKeys.count(); i++) {
        m_partitionAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SingleTagSelect.brk", m_archive);
        m_partitionAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        for (u32 j = i == 2 ? 4 : 2; j <= 4; j++) {
            m_mainScreen.search("C%ubar%u", j, i + 1)->setAnimation(m_partitionAnmTevRegKeys[i]);
            for (u32 k = 0; k < j; k++) {
                m_mainScreen.search("C%upm%u", j, i * j + k + 1)
                        ->setAnimation(m_partitionAnmTevRegKeys[i]);
                m_mainScreen.search("C%ucha%u", j, i * j + k + 1)
                        ->setAnimation(m_partitionAnmTevRegKeys[i]);
            }
            for (u32 k = GetFirstStatusIndex(j, i); k < GetFirstStatusIndex(j, i + 1); k++) {
                if (j == 2 && i == 1) {
                    m_mainScreen.search("CNm22p")->setAnimation(m_partitionAnmTevRegKeys[i]);
                } else {
                    m_mainScreen.search("C%upb%u", j, k)->setAnimation(m_partitionAnmTevRegKeys[i]);
                }
                if (k + 1 != GetFirstStatusIndex(j, i + 1)) {
                    if (j == 2 && i == 0) {
                        m_mainScreen.search("C2pvsm1")->setAnimation(m_partitionAnmTevRegKeys[i]);
                    } else if (j == 3 && i == 1) {
                        m_mainScreen.search("C3vsm4")->setAnimation(m_partitionAnmTevRegKeys[i]);
                    } else {
                        m_mainScreen.search("C%uvsm%u", j, k - i)
                                ->setAnimation(m_partitionAnmTevRegKeys[i]);
                    }
                }
            }
        }
    }
    for (u32 i = 0; i < m_partitionCircleAnmTransforms.count(); i++) {
        m_partitionCircleAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("SingleTagSelect.bck", m_archive);
        for (u32 j = i == 2 ? 4 : 2; j <= 4; j++) {
            for (u32 k = GetFirstStatusIndex(j, i); k < GetFirstStatusIndex(j, i + 1); k++) {
                if (j == 2 && i == 1) {
                    m_mainScreen.search("CNm22p")->setAnimation(m_partitionCircleAnmTransforms[i]);
                } else {
                    m_mainScreen.search("C%upb%u", j, k)
                            ->setAnimation(m_partitionCircleAnmTransforms[i]);
                }
            }
        }
    }
    for (u32 i = 0; i < m_partitionWindowAnmTransforms.count(); i++) {
        m_partitionWindowAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("SingleTagSelect.bck", m_archive);
        if (i != 2) {
            for (u32 j = GetFirstStatusIndex(2, i); j < GetFirstStatusIndex(2, i + 1); j++) {
                m_mainScreen.search("C2chaw%uN", j)
                        ->setAnimation(m_partitionWindowAnmTransforms[i]);
            }
        }
        for (u32 j = i == 2 ? 4 : 3; j <= 4; j++) {
            m_mainScreen.search("CPcN%u%u", j, i + 1)
                    ->setAnimation(m_partitionWindowAnmTransforms[i]);
        }
    }
    m_padCountAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_padCountScreen.search("N_Stok")->setAnimation(m_padCountAnmTransform);
    m_padCountCircleAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_padCountScreen.search("Cstok_pb")->setAnimation(m_padCountCircleAnmTransform);

    m_mainAnmTextureSRTKeyFrame = 0;
    m_mainAnmColorFrame = 0;
    m_partitionAnmTransformFrames.fill(14);
    m_partitionAnmTextureSRTKeyFrames.fill(0);
    m_partitionAnmTevRegKeyFrames.fill(0);
    m_partitionCircleAnmTransformFrames.fill(14);
    m_partitionWindowAnmTransformFrames.fill(10);
    m_padCountCircleAnmTransformFrame = 0;
}

SceneTandemSelect::~SceneTandemSelect() {}

void SceneTandemSelect::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::NameSelect) {
        m_padCount = SequenceInfo::Instance().m_padCount;
        m_partitionCount = m_padCount == 4 ? 3 : 2;
        m_partitionIndex = 0;

        for (u32 i = 2; i <= 4; i++) {
            m_mainScreen.search("N_C%up", i)->m_isVisible = i == m_padCount;
        }

        J2DPicture *picture = m_padCountScreen.search("Cstok_p")->downcast<J2DPicture>();
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "Player%lu.bti", m_padCount);
        picture->changeTexture(name.values(), 0);
    }

    slideIn();
}

void SceneTandemSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_padCountScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneTandemSelect::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_mainAnmTextureSRTKeyFrame = (m_mainAnmTextureSRTKeyFrame + 1) % 180;
    m_mainAnmColorFrame = (m_mainAnmColorFrame + 1) % 120;
    for (u32 i = 0; i < 3; i++) {
        if (i == m_partitionIndex) {
            if (m_partitionAnmTransformFrames[i] < 22) {
                m_partitionAnmTransformFrames[i]++;
            }
            m_partitionAnmTextureSRTKeyFrames[i] = (m_partitionAnmTextureSRTKeyFrames[i] + 1) % 180;
            m_partitionAnmTevRegKeyFrames[i] = 1;
            m_partitionCircleAnmTransformFrames[i] =
                    14 + (m_partitionCircleAnmTransformFrames[i] - 13) % 60;
            m_partitionWindowAnmTransformFrames[i] =
                    10 + (m_partitionWindowAnmTransformFrames[i] - 9) % 49;
        } else {
            if (m_partitionAnmTransformFrames[i] > 14) {
                m_partitionAnmTransformFrames[i]--;
            }
            m_partitionAnmTextureSRTKeyFrames[i] = 0;
            m_partitionAnmTevRegKeyFrames[i] = 0;
            m_partitionCircleAnmTransformFrames[i] = 0;
            m_partitionWindowAnmTransformFrames[i] = 0;
        }
    }
    m_padCountCircleAnmTransformFrame = 14 + (m_padCountCircleAnmTransformFrame - 13) % 60;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_mainAnmTextureSRTKey->m_frame = m_mainAnmTextureSRTKeyFrame;
    m_mainAnmColor->m_frame = m_mainAnmColorFrame;
    for (u32 i = 0; i < m_partitionAnmTransforms.count(); i++) {
        m_partitionAnmTransforms[i]->m_frame = m_partitionAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_partitionAnmTextureSRTKeys.count(); i++) {
        m_partitionAnmTextureSRTKeys[i]->m_frame = m_partitionAnmTextureSRTKeyFrames[i];
    }
    for (u32 i = 0; i < m_partitionAnmTevRegKeys.count(); i++) {
        m_partitionAnmTevRegKeys[i]->m_frame = m_partitionAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_partitionCircleAnmTransforms.count(); i++) {
        m_partitionCircleAnmTransforms[i]->m_frame = m_partitionCircleAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_partitionWindowAnmTransforms.count(); i++) {
        m_partitionWindowAnmTransforms[i]->m_frame = m_partitionWindowAnmTransformFrames[i];
    }
    m_padCountAnmTransform->m_frame = m_padCountAnmTransformFrame;
    m_padCountCircleAnmTransform->m_frame = m_padCountCircleAnmTransformFrame;

    m_mainScreen.animation();
    m_padCountScreen.animation();
}

void SceneTandemSelect::slideIn() {
    ServerManager::Instance()->unlock();
    Client::Instance()->reset();
    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::SelectTandem);
    m_mainAnmTransformFrame = 0;
    m_padCountAnmTransformFrame = 0;
    m_state = &SceneTandemSelect::stateSlideIn;
}

void SceneTandemSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneTandemSelect::stateSlideOut;
}

void SceneTandemSelect::idle() {
    m_state = &SceneTandemSelect::stateIdle;
}

void SceneTandemSelect::nextScene() {
    m_state = &SceneTandemSelect::stateNextScene;
}

void SceneTandemSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        if (m_mainAnmTransformFrame <= 9) {
            m_padCountAnmTransformFrame = m_mainAnmTransformFrame;
        }
    } else {
        idle();
    }
}

void SceneTandemSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_mainAnmTransformFrame <= 9) {
            m_padCountAnmTransformFrame = m_mainAnmTransformFrame;
        }
    } else {
        nextScene();
    }
}

void SceneTandemSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::ServerSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        RaceInfo::Instance().m_statusCount = GetStatusCount(m_padCount, m_partitionIndex);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::NameSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        m_partitionIndex = m_partitionIndex == 0 ? m_partitionCount - 1 : m_partitionIndex - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_partitionIndex = m_partitionIndex == m_partitionCount - 1 ? 0 : m_partitionIndex + 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneTandemSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

u32 SceneTandemSelect::GetStatusCount(u32 padCount, u32 partitionIndex) {
    if (partitionIndex == 0) {
        return padCount;
    }

    if (padCount == 4 && partitionIndex == 1) {
        return 2;
    }

    return padCount - 1;
}

u32 SceneTandemSelect::GetFirstStatusIndex(u32 padCount, u32 partitionIndex) {
    u32 firstStatusIndex = 1;
    for (u32 i = 0; i < partitionIndex; i++) {
        firstStatusIndex += GetStatusCount(padCount, i);
    }
    return firstStatusIndex;
}
