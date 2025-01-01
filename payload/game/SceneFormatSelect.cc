#include "SceneFormatSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>

SceneFormatSelect::SceneFormatSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *mapSelectArchive = sceneFactory->archive(SceneFactory::ArchiveType::MapSelect);

    m_mainScreen.set("SelectFormat.blo", 0x1040000, m_archive);
    m_modeScreen.set("SelectMapLayout.blo", 0x1040000, mapSelectArchive);
    for (u32 i = 0; i < m_playerCountScreens.count(); i++) {
        m_playerCountScreens[i].set("PlayerCount.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_playerCountScreens.count(); i++) {
        m_mainScreen.search("M4pc%u", i + 1)->appendChild(&m_playerCountScreens[i]);
    }
    m_modeScreen.search("OK_wb11")->m_isVisible = false;
    m_modeScreen.search("NRandom")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_mainAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("SelectMode.btk", m_archive);
    m_mainAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmTextureSRTKey);
    m_mainAnmColor = J2DAnmLoaderDataBase::Load("SelectMode.bpk", m_archive);
    m_mainAnmColor->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmColor);
    for (u32 i = 0; i < m_formatAnmTevRegKeys.count(); i++) {
        m_formatAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SelectMode.brk", m_archive);
        m_formatAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("M4bar%u", i + 1)->setAnimation(m_formatAnmTevRegKeys[i]);
    }
    m_mainScreen.search("MgpM21")->setAnimation(m_formatAnmTevRegKeys[0]);
    m_mainScreen.search("Mgpm1")->setAnimation(m_formatAnmTevRegKeys[0]);
    m_mainScreen.search("MvsM21")->setAnimation(m_formatAnmTevRegKeys[1]);
    m_mainScreen.search("Mvs")->setAnimation(m_formatAnmTevRegKeys[1]);
    m_mainScreen.search("MminM21")->setAnimation(m_formatAnmTevRegKeys[2]);
    m_mainScreen.search("Mmini")->setAnimation(m_formatAnmTevRegKeys[2]);
    for (u32 i = 0; i < m_cursorAnmTransforms.count(); i++) {
        m_cursorAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
        m_mainScreen.search("M4crsr%u", i + 1)->setAnimation(m_cursorAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_nameAnmTransforms.count(); i++) {
        m_nameAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    }
    m_mainScreen.search("Mgpm1N")->setAnimation(m_nameAnmTransforms[0]);
    m_mainScreen.search("MvsN")->setAnimation(m_nameAnmTransforms[1]);
    m_mainScreen.search("MminiN")->setAnimation(m_nameAnmTransforms[2]);
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    }
    m_mainScreen.search("MgpM21")->setAnimation(m_circleAnmTransforms[0]);
    m_mainScreen.search("MvsM21")->setAnimation(m_circleAnmTransforms[1]);
    m_mainScreen.search("MminM21")->setAnimation(m_circleAnmTransforms[2]);
    m_modeAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", mapSelectArchive);
    m_modeScreen.search("NSlMap")->setAnimation(m_modeAnmTransform);
    for (u32 i = 0; i < m_playerCountAnmTevRegKeys.count(); i++) {
        m_playerCountAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("PlayerCount.brk", m_archive);
        m_playerCountAnmTevRegKeys[i]->searchUpdateMaterialID(&m_playerCountScreens[i]);
        m_playerCountScreens[i].search("PIcon")->setAnimation(m_playerCountAnmTevRegKeys[i]);
        for (u32 j = 1; j <= 3; j++) {
            for (u32 k = 0; k < j; k++) {
                m_playerCountScreens[i]
                        .search("PCount%u%u", j, k)
                        ->setAnimation(m_playerCountAnmTevRegKeys[i]);
            }
        }
    }

    m_mainAnmTextureSRTKeyFrame = 0;
    m_mainAnmColorFrame = 0;
    m_formatAnmTevRegKeyFrames.fill(0);
    m_cursorAnmTransformFrames.fill(10);
    m_nameAnmTransformFrames.fill(14);
    m_circleAnmTransformFrames.fill(10);
    m_playerCountAnmTevRegKeyFrames.fill(0);
}

SceneFormatSelect::~SceneFormatSelect() {}

void SceneFormatSelect::init() {
    J2DPicture *iconPicture = m_modeScreen.search("BtlPict")->downcast<J2DPicture>();
    J2DPicture *namePicture = m_modeScreen.search("SubM")->downcast<J2DPicture>();
    RaceInfo &raceInfo = RaceInfo::Instance();
    switch (raceInfo.m_raceMode) {
    case RaceMode::Balloon:
        iconPicture->changeTexture("Cup_Pict_Balloon.bti", 0);
        namePicture->changeTexture("Mozi_Battle1.bti", 0);
        break;
    case RaceMode::Bomb:
        iconPicture->changeTexture("Cup_Pict_Bomb.bti", 0);
        namePicture->changeTexture("Mozi_Battle3.bti", 0);
        break;
    case RaceMode::Escape:
        iconPicture->changeTexture("Cup_Pict_Shine.bti", 0);
        namePicture->changeTexture("Mozi_Battle2.bti", 0);
        break;
    default:
        iconPicture->changeTexture("Cup_Pict_LAN.bti", 0);
        namePicture->changeTexture("Entry_Versus.bti", 0);
        break;
    }

    slideIn();
}

void SceneFormatSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_modeScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneFormatSelect::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    refreshFormats();

    m_mainAnmTextureSRTKeyFrame = (m_mainAnmTextureSRTKeyFrame + 1) % 180;
    m_mainAnmColorFrame = (m_mainAnmColorFrame + 1) % 120;
    for (u32 i = 0; i < FormatCount; i++) {
        if (i == m_formatIndex) {
            m_formatAnmTevRegKeyFrames[i] = 1;
            if (m_cursorAnmTransformFrames[i] < 18) {
                m_cursorAnmTransformFrames[i]++;
            }
            if (m_nameAnmTransformFrames[i] < 19) {
                m_nameAnmTransformFrames[i]++;
            }
            m_circleAnmTransformFrames[i] = 10 + (m_circleAnmTransformFrames[i] - 9) % 60;
            m_playerCountAnmTevRegKeyFrames[i] = 1;
        } else {
            m_formatAnmTevRegKeyFrames[i] = 0;
            if (m_cursorAnmTransformFrames[i] > 10) {
                m_cursorAnmTransformFrames[i]--;
            }
            if (m_nameAnmTransformFrames[i] > 14) {
                m_nameAnmTransformFrames[i]--;
            }
            m_circleAnmTransformFrames[i] = 10;
            m_playerCountAnmTevRegKeyFrames[i] = 0;
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_mainAnmTextureSRTKey->m_frame = m_mainAnmTextureSRTKeyFrame;
    m_mainAnmColor->m_frame = m_mainAnmColorFrame;
    m_modeAnmTransform->m_frame = m_modeAnmTransformFrame;
    for (u32 i = 0; i < m_formatAnmTevRegKeys.count(); i++) {
        m_formatAnmTevRegKeys[i]->m_frame = m_formatAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_cursorAnmTransforms.count(); i++) {
        m_cursorAnmTransforms[i]->m_frame = m_cursorAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_nameAnmTransforms.count(); i++) {
        m_nameAnmTransforms[i]->m_frame = m_nameAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i]->m_frame = m_circleAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_playerCountAnmTevRegKeys.count(); i++) {
        m_playerCountAnmTevRegKeys[i]->m_frame = m_playerCountAnmTevRegKeyFrames[i];
    }

    m_mainScreen.animation();
    m_modeScreen.animation();
    for (u32 i = 0; i < m_playerCountScreens.count(); i++) {
        m_playerCountScreens[i].animationMaterials();
    }
}

void SceneFormatSelect::slideIn() {
    if (SequenceApp::Instance()->prevScene() != SceneType::PlayerList) {
        m_formatIndex = 0;
    }

    MenuTitleLine::Instance()->drop("SelectFormat.bti");
    m_mainAnmTransformFrame = 0;
    m_state = &SceneFormatSelect::stateSlideIn;
}

void SceneFormatSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneFormatSelect::stateSlideOut;
}

void SceneFormatSelect::idle() {
    m_state = &SceneFormatSelect::stateIdle;
}

void SceneFormatSelect::nextScene() {
    m_state = &SceneFormatSelect::stateNextScene;
}

void SceneFormatSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 10) {
        m_mainAnmTransformFrame++;
        m_modeAnmTransformFrame = m_mainAnmTransformFrame;
    } else {
        idle();
    }
}

void SceneFormatSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_modeAnmTransformFrame = m_mainAnmTransformFrame;
    } else {
        nextScene();
    }
}

void SceneFormatSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::PlayerList;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::PackSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        m_formatIndex = m_formatIndex == 0 ? FormatCount - 1 : m_formatIndex - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_formatIndex = m_formatIndex == FormatCount - 1 ? 0 : m_formatIndex + 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneFormatSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneFormatSelect::refreshFormats() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < FormatCount; i++) {
        J2DScreen &screen = m_playerCountScreens[i];
        kart2DCommon->changeNumberTexture(i * 64, 3, screen, "PCount");
    }
}
