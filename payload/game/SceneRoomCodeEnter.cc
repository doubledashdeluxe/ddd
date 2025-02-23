#include "SceneRoomCodeEnter.hh"

#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <common/Algorithm.hh>
#include <jsystem/J2DAnmLoaderDataBase.hh>

SceneRoomCodeEnter::SceneRoomCodeEnter(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *menuArchive = sceneFactory->archive(SceneFactory::ArchiveType::Menu);

    m_mainScreen.set("EnterRoomCode.blo", 0x1040000, m_archive);
    m_tandemScreen.set("PlayerIcon.blo", 0x1040000, menuArchive);
    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_charScreens[i].set("EnterRoomCodeChar.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_mainScreen.search("Char%u", i)->appendChild(&m_charScreens[i]);
    }

    m_tandemScreen.search("Cstok_pb")->m_isVisible = false;
    m_tandemScreen.search("Cstok_p")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("EnterRoomCode.bck", m_archive);
    m_mainScreen.search("NSlMap")->setAnimation(m_mainAnmTransform);
    m_selectAnmTransform = J2DAnmLoaderDataBase::Load("EnterRoomCode.bck", m_archive);
    m_mainScreen.search("NRandom")->setAnimation(m_selectAnmTransform);
    m_mainScreen.search("OK_wb11")->setAnimation(m_selectAnmTransform);
    m_tandemAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", menuArchive);
    m_tandemScreen.search("N_Stok")->setAnimation(m_tandemAnmTransform);
    m_tandemCircleAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", menuArchive);
    for (u32 i = 1; i <= 9; i++) {
        m_tandemScreen.search("SMpb%u", i)->setAnimation(m_tandemCircleAnmTransform);
    }
    for (u32 i = 0; i < m_charAnmTransforms.count(); i++) {
        m_charAnmTransforms[i] = J2DAnmLoaderDataBase::Load("EnterRoomCodeChar.bck", m_archive);
        m_charScreens[i].setAnimation(m_charAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_charAnmTextureSRTKeys.count(); i++) {
        m_charAnmTextureSRTKeys[i] = J2DAnmLoaderDataBase::Load("EnterRoomCodeChar.btk", m_archive);
        m_charAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_charScreens[i]);
        m_charScreens[i].setAnimation(m_charAnmTextureSRTKeys[i]);
    }
    for (u32 i = 0; i < m_charAnmTexPatterns.count(); i++) {
        m_charAnmTexPatterns[i] = J2DAnmLoaderDataBase::Load("EnterRoomCodeChar.btp", m_archive);
        m_charAnmTexPatterns[i]->searchUpdateMaterialID(&m_charScreens[i]);
        m_charScreens[i].setAnimation(m_charAnmTexPatterns[i]);
    }

    m_tandemCircleAnmTransformFrame = 0;
    m_charAnmTransformFrames.fill(0);
    m_charAnmTextureSRTKeyFrames.fill(0);
    m_charAnmTexPatternFrames.fill(0);
    m_charAlphas.fill(0);
}

SceneRoomCodeEnter::~SceneRoomCodeEnter() {}

void SceneRoomCodeEnter::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::RoomTypeSelect) {
        m_padCount = SequenceInfo::Instance().m_padCount;
        m_statusCount = RaceInfo::Instance().m_statusCount;
        m_charCount = 0;
        m_chars.fill(0);

        m_tandemScreen.search("Ns1234")->m_isVisible = m_statusCount + 0 == m_padCount;
        m_tandemScreen.search("Ns12_3_4")->m_isVisible = m_statusCount + 1 == m_padCount;
        m_tandemScreen.search("Ns12_34")->m_isVisible = m_statusCount + 2 == m_padCount;
        for (u32 i = 0, j = 1; i < 3; i++) {
            for (u32 k = 1; k < 4 - i; j++, k++) {
                m_tandemScreen.search("SMm%u", j)->m_isVisible = m_statusCount > k;
                m_tandemScreen.search("SMpb%u", 1 + i + j)->m_isVisible = m_statusCount > k;
            }
            for (u32 k = 1; k < 4; k++) {
                m_tandemScreen.search("SMpm%u", 1 + i * 4 + k)->m_isVisible = m_padCount > k;
            }
        }
    }

    slideIn();
}

void SceneRoomCodeEnter::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_tandemScreen.draw(0.0f, 0.0f, m_graphContext);
    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_charScreens[i].animationMaterials();
    }
}

void SceneRoomCodeEnter::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    for (u32 i = 0; i < MaxCharCount; i++) {
        if (i < m_charCount) {
            if (m_charAnmTransformFrames[i] < 5) {
                m_charAnmTransformFrames[i]++;
            }
            if (m_charAlphas[i] < 255) {
                m_charAlphas[i] += 51;
            }
        } else {
            if (m_charAnmTransformFrames[i] > 0) {
                m_charAnmTransformFrames[i]--;
            }
            if (m_charAlphas[i] > 0) {
                m_charAlphas[i] -= 51;
            }
        }
        m_charAnmTextureSRTKeyFrames[i] = m_chars[i];
        m_charAnmTexPatternFrames[i] = m_chars[i];
    }
    m_tandemCircleAnmTransformFrame = 14 + (m_tandemCircleAnmTransformFrame - 13) % 60;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_selectAnmTransform->m_frame = m_selectAnmTransformFrame;
    m_tandemAnmTransform->m_frame = m_tandemAnmTransformFrame;
    m_tandemCircleAnmTransform->m_frame = m_tandemCircleAnmTransformFrame;
    for (u32 i = 0; i < m_charAnmTransforms.count(); i++) {
        m_charAnmTransforms[i]->m_frame = m_charAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_charAnmTextureSRTKeys.count(); i++) {
        m_charAnmTextureSRTKeys[i]->m_frame = m_charAnmTextureSRTKeyFrames[i];
    }
    for (u32 i = 0; i < m_charAnmTexPatterns.count(); i++) {
        m_charAnmTexPatterns[i]->m_frame = m_charAnmTexPatternFrames[i];
    }

    for (u32 i = 0; i < m_charAlphas.count(); i++) {
        m_charScreens[i].search("SlotB1")->setAlpha(m_charAlphas[i]);
        m_charScreens[i].search("Name1")->setAlpha(m_charAlphas[i]);
        m_charScreens[i].search("Slot")->setAlpha(m_charAlphas[i]);
        m_charScreens[i].search("Light1")->setAlpha(m_charAlphas[i]);
        m_charScreens[i].search("Light2")->setAlpha(m_charAlphas[i]);
    }

    m_mainScreen.animation();
    m_tandemScreen.animation();
    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_charScreens[i].animationMaterials();
    }
}

void SceneRoomCodeEnter::slideIn() {
    MenuTitleLine::Instance()->drop("EnterRoomCode.bti");
    m_mainAnmTransformFrame = 0;
    m_selectAnmTransformFrame = 0;
    m_tandemAnmTransformFrame = 0;
    m_state = &SceneRoomCodeEnter::stateSlideIn;
}

void SceneRoomCodeEnter::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneRoomCodeEnter::stateSlideOut;
}

void SceneRoomCodeEnter::idle() {
    m_state = &SceneRoomCodeEnter::stateIdle;
}

void SceneRoomCodeEnter::selectIn() {
    m_selectAnmTransformFrame = 1;
    m_state = &SceneRoomCodeEnter::stateSelectIn;
}

void SceneRoomCodeEnter::selectOut() {
    m_selectAnmTransformFrame = 8;
    m_state = &SceneRoomCodeEnter::stateSelectOut;
}

void SceneRoomCodeEnter::select() {
    m_state = &SceneRoomCodeEnter::stateSelect;
}

void SceneRoomCodeEnter::nextScene() {
    m_state = &SceneRoomCodeEnter::stateNextScene;
}

void SceneRoomCodeEnter::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        m_tandemAnmTransformFrame = Min<u8>(m_mainAnmTransformFrame, 9);
    } else {
        idle();
    }
}

void SceneRoomCodeEnter::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_tandemAnmTransformFrame = Min<u8>(m_mainAnmTransformFrame, 9);
    } else {
        nextScene();
    }
}

void SceneRoomCodeEnter::stateIdle() {
    u32 charCount = m_charCount;

    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_chars[m_charCount++] = 8;
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        if (m_charCount == 0) {
            m_nextScene = SceneType::RoomTypeSelect;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
            slideOut();
        } else {
            m_charCount--;
        }
    } else if (button.risingEdge() & PAD_BUTTON_X) {
        m_chars[m_charCount++] = 0;
    } else if (button.risingEdge() & PAD_BUTTON_Y) {
        m_chars[m_charCount++] = 1;
    } else if (button.risingEdge() & PAD_TRIGGER_L) {
        m_chars[m_charCount++] = 2;
    } else if (button.risingEdge() & PAD_TRIGGER_R) {
        m_chars[m_charCount++] = 3;
    } else if (button.risingEdge() & JUTGamePad::PAD_MSTICK_UP) {
        m_chars[m_charCount++] = 4;
    } else if (button.risingEdge() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_chars[m_charCount++] = 5;
    } else if (button.risingEdge() & JUTGamePad::PAD_MSTICK_LEFT) {
        m_chars[m_charCount++] = 6;
    } else if (button.risingEdge() & JUTGamePad::PAD_MSTICK_RIGHT) {
        m_chars[m_charCount++] = 7;
    } else if (button.risingEdge() & PAD_TRIGGER_Z) {
        m_nextScene = SceneType::ModeSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    }

    if (m_charCount != charCount) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
    }

    if (m_chars[m_charCount - 1] == 8 || m_charCount == MaxCharCount) {
        selectIn();
    }
}

void SceneRoomCodeEnter::stateSelectIn() {
    m_selectAnmTransformFrame++;
    if (m_selectAnmTransformFrame == 9) {
        select();
    }
}

void SceneRoomCodeEnter::stateSelectOut() {
    m_selectAnmTransformFrame--;
    if (m_selectAnmTransformFrame == 0) {
        idle();
    }
}

void SceneRoomCodeEnter::stateSelect() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::PersonalRoom;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        RaceInfo::Instance().m_raceMode = RaceMode::VS;
        SequenceInfo::Instance().m_packIndex = 0;
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_charCount--;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        selectOut();
    }
}

void SceneRoomCodeEnter::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
