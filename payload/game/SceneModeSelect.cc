#include "SceneModeSelect.hh"

#include "game/ErrorViewApp.hh"
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
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/Client.hh>
#include <portable/Formatter.hh>

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

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        J2DPicture *iconPicture = m_modeScreens[i].search("Icon")->downcast<J2DPicture>();
        const char *iconTextureName = RaceMode::IconTextureName(Modes[i]);
        iconPicture->changeTexture(iconTextureName, 0);
        Array<char, 32> path;
        snprintf(path.values(), path.count(), "/modenames/%lu.txt", Modes[i]);
        char *name = static_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path.values()));
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
    Client *client = Client::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    u32 increment = 5;
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.level() & (PAD_TRIGGER_L | PAD_TRIGGER_R)) {
        increment += 20;
    }
    for (u32 i = 0; i < m_descOffsets.count(); i++) {
        m_descOffsets[i] += increment;
    }
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
        for (s32 j = 0; j < 48; j++) {
            u8 alpha = 255;
            if (j == 0) {
                alpha = 255 - m_descAlphas[i];
            } else if (j == 47 - (m_roomType == RoomType::Worldwide) * 6) {
                alpha = m_descAlphas[i];
            } else if (j > 47 - (m_roomType == RoomType::Worldwide) * 6) {
                alpha = 0;
            }
            m_modeScreens[i].search("Desc%u", j)->setAlpha(alpha);
        }
    }

    m_mainScreen.animation();
    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        m_modeScreens[i].animationMaterials();
    }

    ClientStateModeWriteInfo writeInfo;
    writeInfo.playerCount = SequenceInfo::Instance().m_padCount;
    writeInfo.serverIndex = OnlineInfo::Instance().m_serverIndex;
    client->writeStateMode(writeInfo);
}

SceneModeSelect::DescText::DescText(SceneModeSelect &scene, u32 descIndex)
    : m_scene(scene), m_descIndex(descIndex) {}

SceneModeSelect::DescText::~DescText() {}

const char *SceneModeSelect::DescText::getPart(u32 /* partIndex */) {
    return m_scene.m_descs[m_descIndex].values();
}

void SceneModeSelect::DescText::setAnmTransformFrame(u8 anmTransformFrame) {
    m_scene.m_descAnmTransformFrames[m_descIndex] = anmTransformFrame;
}

void SceneModeSelect::DescText::setAlpha(u8 alpha) {
    m_scene.m_descAlphas[m_descIndex] = alpha;
}

bool SceneModeSelect::clientStateServer(const ClientStateServerReadInfo & /* readInfo */) {
    return true;
}

bool SceneModeSelect::clientStateMode(const ClientStateModeReadInfo &readInfo) {
    if (readInfo.modes) {
        const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
        const OnlineInfo &onlineInfo = OnlineInfo::Instance();
        for (u32 i = 0; i < ModeCount; i++) {
            const ClientStateModeReadInfo::Mode &mode = (*readInfo.modes)[i];
            Array<char, 80> &desc = m_descs[i];
            Formatter formatter(desc);
            for (u32 j = 0; j < sequenceInfo.m_padCount; j++) {
                if (j != 0) {
                    formatter.printf(" / ");
                }
                formatter.printf("%s: %u MMR", onlineInfo.m_names[j], mode.mmrs[j]);
            }
            Array<char, 4> &playerCount = m_playerCounts[i];
            u32 uncappedPlayerCount = mode.playerCount;
            u16 cappedPlayerCount = Min<u16>(uncappedPlayerCount, 999);
            snprintf(playerCount.values(), playerCount.count(), "%u", cappedPlayerCount);
        }
    }
    return true;
}

void SceneModeSelect::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneModeSelect::slideIn() {
    if (SequenceApp::Instance()->prevScene() != SceneType::PackSelect) {
        m_roomType = OnlineInfo::Instance().m_roomType;
        m_modeIndex = 0;
    }
    for (u32 i = 0; i < m_descs.count(); i++) {
        snprintf(m_descs[i].values(), m_descs[i].count(), "%s", String(9));
    }
    m_descOffsets.fill(0);
    for (u32 i = 0; i < m_playerCounts.count(); i++) {
        snprintf(m_playerCounts[i].values(), m_playerCounts[i].count(), "...");
    }

    for (u32 i = 0; i < m_modeScreens.count(); i++) {
        m_modeScreens[i].search("PIcon")->m_isVisible = m_roomType == RoomType::Worldwide;
        m_modeScreens[i].search("PCount")->m_isVisible = m_roomType == RoomType::Worldwide;
    }

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
        if (m_roomType == RoomType::Worldwide) {
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
        u64 descOffset = Max<u64>(m_descOffsets[i], 300) - 300;
        u32 descPictureCount = 48 - (m_roomType == RoomType::Worldwide) * 6;
        descText.refresh(descOffset, 1, descPictureCount, screen, "Desc");
        kart2DCommon->changeUnicodeTexture(m_playerCounts[i].values(), 3, screen, "PCount", true);
    }
}

const char *SceneModeSelect::String(u32 index) {
    return SceneFactory::Instance()->string(SceneFactory::ArchiveType::LanEntry, index);
}

const Array<u32, SceneModeSelect::ModeCount> SceneModeSelect::Modes((u32[ModeCount]){
        RaceMode::VS,
        RaceMode::Balloon,
        RaceMode::Escape,
        RaceMode::Bomb,
        RaceMode::TA,
});
