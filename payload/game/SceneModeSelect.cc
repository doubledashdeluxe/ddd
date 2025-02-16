#include "SceneModeSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/RoomType.hh"
#include "game/SequenceApp.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>

extern "C" {
#include <stdio.h>
}

SceneModeSelect::SceneModeSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_mainScreen.set("GDIndexLayout.blo", 0x20000, m_archive);
    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        m_modeScreens[i].set("Mode.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        m_modeScreens[i].search("GDCurs")->setHasARTrans(false, true);
        m_modeScreens[i].search("GDCurs")->setHasARShift(false, true);
        m_modeScreens[i].search("GDCurs")->setHasARScale(false, true);
    }

    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        m_mainScreen.search("Num%02u", i)->appendChild(&m_modeScreens[i]);
    }
    m_mainScreen.search("NSaveGD")->m_isVisible = false;
    for (u32 i = 0; i < 2; i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->m_isVisible = false;
    }

    Array<const char *, ModeCount> iconNames;
    iconNames[0] = "Cup_Pict_LAN.bti";
    iconNames[1] = "Cup_Pict_Balloon.bti";
    iconNames[2] = "Cup_Pict_Shine.bti";
    iconNames[3] = "Cup_Pict_Bomb.bti";
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        J2DPicture *iconPicture = m_modeScreens[i].search("Icon")->downcast<J2DPicture>();
        iconPicture->changeTexture(iconNames[i], 0);
        Array<char, 32> path;
        snprintf(path.values(), path.count(), "/modenames/%u.txt", Modes[i]);
        char *name =
                reinterpret_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path.values()));
        u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, name);
        name[size - 1] = '\0';
        kart2DCommon->changeUnicodeTexture(name, 24, m_modeScreens[i], "Name");
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectServerLayout.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    for (u32 i = 0; i < m_modeAnmTransforms.count(); i++) {
        m_modeAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_modeScreens[i].setAnimation(m_modeAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_descAnmTransforms.count(); i++) {
        m_descAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_modeScreens[i].search("Desc")->setAnimation(m_descAnmTransforms[i]);
    }

    m_modeAnmTransformFrames.fill(0);
    m_descAnmTransformFrames.fill(0);
    m_descAlphas.fill(0);
}

SceneModeSelect::~SceneModeSelect() {}

void SceneModeSelect::init() {
    slideIn();
}

void SceneModeSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneModeSelect::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_descOffset += 5;
    refreshModes();

    for (u32 i = 0; i < ModeCount; i++) {
        if (i == m_modeIndex) {
            if (m_modeAnmTransformFrames[i] < 7) {
                m_modeAnmTransformFrames[i]++;
            }
        } else {
            if (m_modeAnmTransformFrames[i] > 0) {
                m_modeAnmTransformFrames[i]--;
            }
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    for (u32 i = 0; i < m_modeAnmTransforms.count(); i++) {
        m_modeAnmTransforms[i]->m_frame = m_modeAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_descAnmTransforms.count(); i++) {
        m_descAnmTransforms[i]->m_frame = m_descAnmTransformFrames[i];
    }

    for (u32 i = 0; i < m_descAlphas.count(); i++) {
        m_modeScreens[i].search("Desc0")->setAlpha(255 - m_descAlphas[i]);
        m_modeScreens[i].search("Desc41")->setAlpha(m_descAlphas[i]);
    }

    m_mainScreen.animation();
    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        m_modeScreens[i].animationMaterials();
    }
}

SceneModeSelect::DescText::DescText(SceneModeSelect &scene, u32 descIndex)
    : m_scene(scene), m_descIndex(descIndex) {}

SceneModeSelect::DescText::~DescText() {}

const char *SceneModeSelect::DescText::getPart(u32 /* partIndex */) {
    return "ABC - 1234";
}

void SceneModeSelect::DescText::setAnmTransformFrame(u8 anmTransformFrame) {
    m_scene.m_descAnmTransformFrames[m_descIndex] = anmTransformFrame;
}

void SceneModeSelect::DescText::setAlpha(u8 alpha) {
    m_scene.m_descAlphas[m_descIndex] = alpha;
}

void SceneModeSelect::slideIn() {
    if (SequenceApp::Instance()->prevScene() != SceneType::PackSelect) {
        m_modeIndex = 0;
    }
    m_descOffset = 0;

    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::SelectMode);
    m_mainAnmTransformFrame = 0;
    m_state = &SceneModeSelect::stateSlideIn;
}

void SceneModeSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_mainAnmTransformFrame = 30;
    m_state = &SceneModeSelect::stateSlideOut;
}

void SceneModeSelect::idle() {
    m_state = &SceneModeSelect::stateIdle;
}

void SceneModeSelect::nextScene() {
    m_state = &SceneModeSelect::stateNextScene;
}

void SceneModeSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 30) {
        m_mainAnmTransformFrame++;
    } else {
        idle();
    }
}

void SceneModeSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
    } else {
        nextScene();
    }
}

void SceneModeSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::PackSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        RaceInfo::Instance().m_raceMode = Modes[m_modeIndex];
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        if (OnlineInfo::Instance().m_roomType == RoomType::Worldwide) {
            m_nextScene = SceneType::RoomTypeSelect;
        } else {
            m_nextScene = SceneType::RoomCodeEnter;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_modeIndex >= 1) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            m_modeIndex--;
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_modeIndex + 1 < ModeCount) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            m_modeIndex++;
        }
    }
}

void SceneModeSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneModeSelect::refreshModes() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < ModeCount; i++) {
        J2DScreen &screen = m_modeScreens[i];
        DescText descText(*this, i);
        descText.refresh(m_descOffset, 4, 42, screen, "Desc");
        kart2DCommon->changeNumberTexture(567, 3, screen, "PCount");
    }
}

const Array<u32, SceneModeSelect::ModeCount> SceneModeSelect::Modes((u32[ModeCount]){
        RaceMode::VS,
        RaceMode::Balloon,
        RaceMode::Escape,
        RaceMode::Bomb,
});
