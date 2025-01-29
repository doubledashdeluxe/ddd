#include "SceneHowManyPlayers.hh"

#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/RaceInfo.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

SceneHowManyPlayers::SceneHowManyPlayers(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);

    OnlineBackground::Create(lanEntryArchive);
    MenuTitleLine::Create(titleLineArchive, heap);

    m_screen.set("HowManyDrivers.blo", 0x1040000, m_archive);

    m_anmTransform = J2DAnmLoaderDataBase::Load("HowManyDrivers.bck", m_archive);
    m_screen.search("N_Entry")->setAnimation(m_anmTransform);
    m_anmTextureSRTKey = J2DAnmLoaderDataBase::Load("HowManyDrivers.btk", m_archive);
    m_anmTextureSRTKey->searchUpdateMaterialID(&m_screen);
    m_screen.setAnimation(m_anmTextureSRTKey);
    m_anmColor = J2DAnmLoaderDataBase::Load("HowManyDrivers.bpk", m_archive);
    m_anmColor->searchUpdateMaterialID(&m_screen);
    m_screen.setAnimation(m_anmColor);
    for (u32 i = 0; i < m_countAnmTransforms.count(); i++) {
        m_countAnmTransforms[i] = J2DAnmLoaderDataBase::Load("HowManyDrivers.bck", m_archive);
        m_screen.search("Ecrsr%u", i + 1)->setAnimation(m_countAnmTransforms[i]);
        m_screen.search("ENplay%u", i + 1)->setAnimation(m_countAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_countAnmTevRegKeys.count(); i++) {
        m_countAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("HowManyDrivers.brk", m_archive);
        m_countAnmTevRegKeys[i]->searchUpdateMaterialID(&m_screen);
        m_screen.search("Ebar%u", i + 1)->setAnimation(m_countAnmTevRegKeys[i]);
        m_screen.search("Eplay%u", i + 1)->setAnimation(m_countAnmTevRegKeys[i]);
        m_screen.search("Eplay%ub", i + 1)->setAnimation(m_countAnmTevRegKeys[i]);
        for (u32 j = 1; j <= i + 1; j++) {
            m_screen.search("Emario%u%u", i + 1, j)->setAnimation(m_countAnmTevRegKeys[i]);
        }
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i] = J2DAnmLoaderDataBase::Load("HowManyDrivers.bck", m_archive);
        m_screen.search("Eplay%ub", i + 1)->setAnimation(m_circleAnmTransforms[i]);
    }

    m_anmTextureSRTKeyFrame = 0;
    m_anmColorFrame = 0;
    m_countAnmTransformFrames.fill(14);
    m_countAnmTevRegKeyFrames.fill(0);
    m_circleAnmTransformFrames.fill(14);
}

SceneHowManyPlayers::~SceneHowManyPlayers() {}

void SceneHowManyPlayers::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::Title) {
        m_padCount = 1;
        System::GetDisplay()->startFadeIn(15);
        GameAudio::Main::Instance()->startSequenceBgm(SoundID::JA_BGM_SELECT);
    }

    slideIn();
}

void SceneHowManyPlayers::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_screen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneHowManyPlayers::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_anmTextureSRTKeyFrame = (m_anmTextureSRTKeyFrame + 1) % 180;
    m_anmColorFrame = (m_anmColorFrame + 1) % 120;
    for (u32 i = 0; i < 4; i++) {
        if (i == m_padCount - 1) {
            if (m_countAnmTransformFrames[i] < 22) {
                m_countAnmTransformFrames[i]++;
            }
            m_countAnmTevRegKeyFrames[i] = 1;
            m_circleAnmTransformFrames[i] = 14 + (m_circleAnmTransformFrames[i] - 13) % 61;
        } else {
            if (m_countAnmTransformFrames[i] > 14) {
                m_countAnmTransformFrames[i]--;
            }
            m_countAnmTevRegKeyFrames[i] = 0;
            m_circleAnmTransformFrames[i] = 14;
        }
    }

    m_anmTransform->m_frame = m_anmTransformFrame;
    m_anmTextureSRTKey->m_frame = m_anmTextureSRTKeyFrame;
    m_anmColor->m_frame = m_anmColorFrame;
    for (u32 i = 0; i < m_countAnmTransforms.count(); i++) {
        m_countAnmTransforms[i]->m_frame = m_countAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_countAnmTevRegKeys.count(); i++) {
        m_countAnmTevRegKeys[i]->m_frame = m_countAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i]->m_frame = m_circleAnmTransformFrames[i];
    }

    m_screen.animation();
}

void SceneHowManyPlayers::slideIn() {
    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::HowManyPlayers);
    m_anmTransformFrame = 0;
    m_state = &SceneHowManyPlayers::stateSlideIn;
}

void SceneHowManyPlayers::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneHowManyPlayers::stateSlideOut;
}

void SceneHowManyPlayers::idle() {
    m_state = &SceneHowManyPlayers::stateIdle;
}

void SceneHowManyPlayers::nextScene() {
    m_state = &SceneHowManyPlayers::stateNextScene;
}

void SceneHowManyPlayers::stateSlideIn() {
    if (m_anmTransformFrame < 15) {
        m_anmTransformFrame++;
    } else {
        idle();
    }
}

void SceneHowManyPlayers::stateSlideOut() {
    if (m_anmTransformFrame > 0) {
        m_anmTransformFrame--;
    } else {
        nextScene();
    }
}

void SceneHowManyPlayers::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::NameSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        SequenceInfo::Instance().m_padCount = m_padCount;
        if (m_padCount == 1) {
            RaceInfo::Instance().m_statusCount = 1;
        }
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::Title;
        GameAudio::Main::Instance()->fadeOutAll(15);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL);
        System::GetDisplay()->startFadeOut(15);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        m_padCount = m_padCount == 1 ? 4 : m_padCount - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_padCount = m_padCount == 4 ? 1 : m_padCount + 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneHowManyPlayers::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
