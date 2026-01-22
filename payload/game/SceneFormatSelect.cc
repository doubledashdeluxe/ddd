#include "SceneFormatSelect.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>
#include <payload/CourseManager.hh>
#include <payload/online/CubeClient.hh>

SceneFormatSelect::SceneFormatSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_mainScreen.set("SelectFormat.blo", 0x1040000, m_archive);
    for (u32 i = 0; i < m_playerCountScreens.count(); i++) {
        m_playerCountScreens[i].set("PlayerCount.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_playerCountScreens.count(); i++) {
        m_mainScreen.search("NPCount%u", i + 1)->appendChild(&m_playerCountScreens[i]);
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectFormat.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_mainAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("HowManyDrivers.btk", m_archive);
    m_mainAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmTextureSRTKey);
    m_mainAnmColor = J2DAnmLoaderDataBase::Load("HowManyDrivers.bpk", m_archive);
    m_mainAnmColor->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmColor);
    m_formatCountAnmTransform = J2DAnmLoaderDataBase::Load("SelectFormat.bck", m_archive);
    for (u32 i = 0; i < FormatCount; i++) {
        m_mainScreen.search("Format%u", i + 1)->setAnimation(m_formatCountAnmTransform);
    }
    for (u32 i = 0; i < m_formatAnmTransforms.count(); i++) {
        m_formatAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectFormat.bck", m_archive);
        m_mainScreen.search("Ecrsr%u", i + 1)->setAnimation(m_formatAnmTransforms[i]);
        m_mainScreen.search("ENplay%u", i + 1)->setAnimation(m_formatAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_formatAnmTevRegKeys.count(); i++) {
        m_formatAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("HowManyDrivers.brk", m_archive);
        m_formatAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("Ebar%u", i + 1)->setAnimation(m_formatAnmTevRegKeys[i]);
        m_mainScreen.search("Eplay%u", i + 1)->setAnimation(m_formatAnmTevRegKeys[i]);
        m_mainScreen.search("Eplay%ub", i + 1)->setAnimation(m_formatAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i] = J2DAnmLoaderDataBase::Load("HowManyDrivers.bck", m_archive);
        m_mainScreen.search("Eplay%ub", i + 1)->setAnimation(m_circleAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_playerCountAnmTevRegKeys.count(); i++) {
        m_playerCountAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("PlayerCount.brk", m_archive);
        m_playerCountAnmTevRegKeys[i]->searchUpdateMaterialID(&m_playerCountScreens[i]);
        m_playerCountScreens[i].search("PIcon")->setAnimation(m_playerCountAnmTevRegKeys[i]);
        for (u32 j = 0; j < 3; j++) {
            m_playerCountScreens[i]
                    .search("PCount%u", j)
                    ->setAnimation(m_playerCountAnmTevRegKeys[i]);
        }
    }

    m_mainAnmTextureSRTKeyFrame = 0;
    m_mainAnmColorFrame = 0;
    m_formatAnmTransformFrames.fill(14);
    m_formatAnmTevRegKeyFrames.fill(0);
    m_circleAnmTransformFrames.fill(14);
    m_playerCountAnmTevRegKeyFrames.fill(0);
}

SceneFormatSelect::~SceneFormatSelect() {}

void SceneFormatSelect::init() {
    slideIn();
}

void SceneFormatSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneFormatSelect::calc() {
    CubeClient *client = CubeClient::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    refreshFormats();

    m_mainAnmTextureSRTKeyFrame = (m_mainAnmTextureSRTKeyFrame + 1) % 180;
    m_mainAnmColorFrame = (m_mainAnmColorFrame + 1) % 120;
    for (u32 i = 0; i < FormatCount; i++) {
        if (i == m_formatIndex) {
            if (m_formatAnmTransformFrames[i] < 22) {
                m_formatAnmTransformFrames[i]++;
            }
            m_formatAnmTevRegKeyFrames[i] = 1;
            m_circleAnmTransformFrames[i] = 14 + (m_circleAnmTransformFrames[i] - 13) % 60;
            m_playerCountAnmTevRegKeyFrames[i] = 1;
        } else {
            if (m_formatAnmTransformFrames[i] > 14) {
                m_formatAnmTransformFrames[i]--;
            }
            m_formatAnmTevRegKeyFrames[i] = 0;
            m_circleAnmTransformFrames[i] = 14;
            m_playerCountAnmTevRegKeyFrames[i] = 0;
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_mainAnmTextureSRTKey->m_frame = m_mainAnmTextureSRTKeyFrame;
    m_mainAnmColor->m_frame = m_mainAnmColorFrame;
    m_formatCountAnmTransform->m_frame = m_formatCountAnmTransformFrame;
    for (u32 i = 0; i < m_formatAnmTransforms.count(); i++) {
        m_formatAnmTransforms[i]->m_frame = m_formatAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_formatAnmTevRegKeys.count(); i++) {
        m_formatAnmTevRegKeys[i]->m_frame = m_formatAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i]->m_frame = m_circleAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_playerCountAnmTevRegKeys.count(); i++) {
        m_playerCountAnmTevRegKeys[i]->m_frame = m_playerCountAnmTevRegKeyFrames[i];
    }

    m_mainScreen.animation();
    for (u32 i = 0; i < m_playerCountScreens.count(); i++) {
        m_playerCountScreens[i].animationMaterials();
    }

    client->writeStatePack(m_writeInfo);
}

bool SceneFormatSelect::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    return true;
}

bool SceneFormatSelect::clientStatePack(const ClientStatePackReadInfo &readInfo) {
    const Optional<ClientStatePackReadInfo::Pack> &pack = readInfo.packs[m_packIndex];
    if (pack) {
        for (u32 i = 0; i < FormatCount; i++) {
            Array<char, 4> &playerCount = m_playerCounts[i];
            u32 uncappedPlayerCount = pack->formatPlayerCounts[i];
            u16 cappedPlayerCount = Min<u16>(uncappedPlayerCount, 999);
            snprintf(playerCount.values(), playerCount.count(), "%u", cappedPlayerCount);
        }
    }
    return true;
}

bool SceneFormatSelect::clientStateRoom(const ClientStateRoomReadInfo & /* readInfo */) {
    return true;
}

bool SceneFormatSelect::clientStateTeam(const ClientStateTeamReadInfo & /* readInfo */) {
    return true;
}

void SceneFormatSelect::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneFormatSelect::slideIn() {
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    m_packIndex = sequenceInfo.m_packIndex;
    if (SequenceApp::Instance()->prevScene() == SceneType::PackSelect) {
        m_formatCount = sequenceInfo.m_statusCount == 1 ? 4 : 3;
        m_formatIndex = 0;

        for (u32 i = 0; i < FormatCount; i++) {
            m_mainScreen.search("Format%u", i + 1)->m_isVisible = i < m_formatCount;
        }
        m_formatCountAnmTransformFrame = m_formatCount;
    }
    for (u32 i = 0; i < m_playerCounts.count(); i++) {
        m_playerCounts[i] = "...";
    }

    const CourseManager *courseManager = CourseManager::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 packCount = courseManager->packCount(true, raceInfo.isRace());
    m_writeInfo.packCount = packCount;
    const CourseManager::Pack &pack = courseManager->pack(true, raceInfo.isRace(), m_packIndex);
    m_writeInfo.packs[m_packIndex].hash = pack.hash();
    m_writeInfo.packIndex = m_packIndex;

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
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
    } else {
        idle();
    }
}

void SceneFormatSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
    } else {
        nextScene();
    }
}

void SceneFormatSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::PersonalRoom;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        OnlineInfo &onlineInfo = OnlineInfo::Instance();
        onlineInfo.m_format = m_formatIndex;
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::PackSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        m_formatIndex = m_formatIndex == 0 ? m_formatCount - 1 : m_formatIndex - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_formatIndex = m_formatIndex == m_formatCount - 1 ? 0 : m_formatIndex + 1;
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
        kart2DCommon->changeUnicodeTexture(m_playerCounts[i].values(), 3, screen, "PCount", true);
    }
}
