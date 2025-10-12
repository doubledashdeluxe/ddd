#include "SceneRoomTypeSelect.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/RoomType.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/Client.hh>
#include <portable/Algorithm.hh>

SceneRoomTypeSelect::SceneRoomTypeSelect(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);

    m_mainScreen.set("SelectRoomType.blo", 0x1040000, lanEntryArchive);
    m_tandemScreen.set("PlayerIcon.blo", 0x1040000, m_archive);

    m_tandemScreen.search("Cstok_pb")->m_isVisible = false;
    m_tandemScreen.search("Cstok_p")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_mainAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("SelectMode.btk", m_archive);
    m_mainAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmTextureSRTKey);
    m_mainAnmColor = J2DAnmLoaderDataBase::Load("SelectMode.bpk", m_archive);
    m_mainAnmColor->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmColor);
    for (u32 i = 0; i < m_roomTypeAnmTevRegKeys.count(); i++) {
        m_roomTypeAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("SelectRoomType.brk", lanEntryArchive);
        m_roomTypeAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
    }
    for (u32 i = 0; i < 5; i++) {
        m_mainScreen.search("M4bar%u", i + 1)
                ->setAnimation(m_roomTypeAnmTevRegKeys[i % MaxRoomTypeCount]);
        m_mainScreen.search("M4rt%u", i + 1)
                ->setAnimation(m_roomTypeAnmTevRegKeys[i % MaxRoomTypeCount]);
    }
    m_mainScreen.search("MgpM21")->setAnimation(m_roomTypeAnmTevRegKeys[0]);
    m_mainScreen.search("Mgpm1")->setAnimation(m_roomTypeAnmTevRegKeys[0]);
    m_mainScreen.search("MvsM21")->setAnimation(m_roomTypeAnmTevRegKeys[1]);
    m_mainScreen.search("Mvs")->setAnimation(m_roomTypeAnmTevRegKeys[1]);
    m_mainScreen.search("MminM21")->setAnimation(m_roomTypeAnmTevRegKeys[2]);
    m_mainScreen.search("Mmini")->setAnimation(m_roomTypeAnmTevRegKeys[2]);
    m_mainScreen.search("MgpM11")->setAnimation(m_roomTypeAnmTevRegKeys[0]);
    m_mainScreen.search("Mgpm2")->setAnimation(m_roomTypeAnmTevRegKeys[0]);
    m_mainScreen.search("MgpM11")->setAnimation(m_roomTypeAnmTevRegKeys[0]);
    m_mainScreen.search("Mgpm2")->setAnimation(m_roomTypeAnmTevRegKeys[0]);
    m_mainScreen.search("MminiM12")->setAnimation(m_roomTypeAnmTevRegKeys[1]);
    m_mainScreen.search("Mmini1")->setAnimation(m_roomTypeAnmTevRegKeys[1]);
    for (u32 i = 0; i < m_cursorAnmTransforms.count(); i++) {
        m_cursorAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    }
    for (u32 i = 0; i < 5; i++) {
        m_mainScreen.search("M4crsr%u", i + 1)
                ->setAnimation(m_cursorAnmTransforms[i % MaxRoomTypeCount]);
    }
    for (u32 i = 0; i < m_nameAnmTransforms.count(); i++) {
        m_nameAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    }
    m_mainScreen.search("Mgpm1N")->setAnimation(m_nameAnmTransforms[0]);
    m_mainScreen.search("MvsN")->setAnimation(m_nameAnmTransforms[1]);
    m_mainScreen.search("MminiN")->setAnimation(m_nameAnmTransforms[2]);
    m_mainScreen.search("MgpmN2")->setAnimation(m_nameAnmTransforms[0]);
    m_mainScreen.search("MminiN1")->setAnimation(m_nameAnmTransforms[1]);
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMode.bck", m_archive);
    }
    m_mainScreen.search("MgpM21")->setAnimation(m_circleAnmTransforms[0]);
    m_mainScreen.search("MvsM21")->setAnimation(m_circleAnmTransforms[1]);
    m_mainScreen.search("MminM21")->setAnimation(m_circleAnmTransforms[2]);
    m_mainScreen.search("MgpM11")->setAnimation(m_circleAnmTransforms[0]);
    m_mainScreen.search("MminiM12")->setAnimation(m_circleAnmTransforms[1]);
    m_tandemAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_tandemScreen.search("N_Stok")->setAnimation(m_tandemAnmTransform);
    m_tandemCircleAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    for (u32 i = 1; i <= 9; i++) {
        m_tandemScreen.search("SMpb%u", i)->setAnimation(m_tandemCircleAnmTransform);
    }

    m_mainAnmTextureSRTKeyFrame = 0;
    m_mainAnmColorFrame = 0;
    m_roomTypeAnmTevRegKeyFrames.fill(0);
    m_cursorAnmTransformFrames.fill(10);
    m_nameAnmTransformFrames.fill(14);
    m_circleAnmTransformFrames.fill(10);
    m_tandemCircleAnmTransformFrame = 0;
}

SceneRoomTypeSelect::~SceneRoomTypeSelect() {}

void SceneRoomTypeSelect::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::ServerSelect) {
        m_padCount = SequenceInfo::Instance().m_padCount;
        m_statusCount = RaceInfo::Instance().m_statusCount;
        m_roomTypeCount = m_statusCount == 1 ? 3 : 2;
        m_roomTypeIndex = 0;

        m_mainScreen.search("N_Mode2")->m_isVisible = m_roomTypeCount == 3;
        m_mainScreen.search("N_Mode1")->m_isVisible = m_roomTypeCount == 2;
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

void SceneRoomTypeSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_tandemScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneRoomTypeSelect::calc() {
    Client *client = Client::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_mainAnmTextureSRTKeyFrame = (m_mainAnmTextureSRTKeyFrame + 1) % 180;
    m_mainAnmColorFrame = (m_mainAnmColorFrame + 1) % 120;
    for (u32 i = 0; i < MaxRoomTypeCount; i++) {
        if (i == m_roomTypeIndex) {
            m_roomTypeAnmTevRegKeyFrames[i] = 1;
            if (m_cursorAnmTransformFrames[i] < 18) {
                m_cursorAnmTransformFrames[i]++;
            }
            if (m_nameAnmTransformFrames[i] < 19) {
                m_nameAnmTransformFrames[i]++;
            }
            m_circleAnmTransformFrames[i] = 10 + (m_circleAnmTransformFrames[i] - 9) % 60;
        } else {
            m_roomTypeAnmTevRegKeyFrames[i] = 0;
            if (m_cursorAnmTransformFrames[i] > 10) {
                m_cursorAnmTransformFrames[i]--;
            }
            if (m_nameAnmTransformFrames[i] > 14) {
                m_nameAnmTransformFrames[i]--;
            }
            m_circleAnmTransformFrames[i] = 10;
        }
    }
    m_tandemCircleAnmTransformFrame = 14 + (m_tandemCircleAnmTransformFrame - 13) % 60;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_mainAnmTextureSRTKey->m_frame = m_mainAnmTextureSRTKeyFrame;
    m_mainAnmColor->m_frame = m_mainAnmColorFrame;
    for (u32 i = 0; i < m_roomTypeAnmTevRegKeys.count(); i++) {
        m_roomTypeAnmTevRegKeys[i]->m_frame = m_roomTypeAnmTevRegKeyFrames[i];
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
    m_tandemAnmTransform->m_frame = m_tandemAnmTransformFrame;
    m_tandemCircleAnmTransform->m_frame = m_tandemCircleAnmTransformFrame;

    m_mainScreen.animation();
    m_tandemScreen.animation();

    ClientStateModeWriteInfo writeInfo;
    writeInfo.playerCount = SequenceInfo::Instance().m_padCount;
    writeInfo.serverIndex = OnlineInfo::Instance().m_serverIndex;
    client->writeStateMode(writeInfo);
}

bool SceneRoomTypeSelect::clientStateServer(const ClientStateServerReadInfo & /* readInfo */) {
    return true;
}

bool SceneRoomTypeSelect::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    return true;
}

void SceneRoomTypeSelect::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneRoomTypeSelect::slideIn() {
    MenuTitleLine::Instance()->drop("SelectRoomType.bti");
    m_mainAnmTransformFrame = 0;
    m_tandemAnmTransformFrame = 0;
    m_state = &SceneRoomTypeSelect::stateSlideIn;
}

void SceneRoomTypeSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneRoomTypeSelect::stateSlideOut;
}

void SceneRoomTypeSelect::idle() {
    m_state = &SceneRoomTypeSelect::stateIdle;
}

void SceneRoomTypeSelect::nextScene() {
    m_state = &SceneRoomTypeSelect::stateNextScene;
}

void SceneRoomTypeSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 10) {
        m_mainAnmTransformFrame++;
        m_tandemAnmTransformFrame = Min<u8>(m_mainAnmTransformFrame, 9);
    } else {
        idle();
    }
}

void SceneRoomTypeSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_tandemAnmTransformFrame = Min<u8>(m_mainAnmTransformFrame, 9);
    } else {
        nextScene();
    }
}

void SceneRoomTypeSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        if (m_roomTypeIndex == 0) {
            m_nextScene = SceneType::ModeSelect;
            OnlineInfo::Instance().m_roomType = RoomType::Worldwide;
        } else if (m_roomTypeIndex == 1 && m_statusCount == 1) {
            m_nextScene = SceneType::PackSelect;
            OnlineInfo::Instance().m_roomType = RoomType::Duel;
            RaceInfo::Instance().m_raceMode = RaceMode::VS;
        } else {
            m_nextScene = SceneType::RoomCodeEnter;
            OnlineInfo::Instance().m_roomType = RoomType::Personal;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::ServerSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        m_roomTypeIndex = m_roomTypeIndex == 0 ? m_roomTypeCount - 1 : m_roomTypeIndex - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_roomTypeIndex = m_roomTypeIndex == m_roomTypeCount - 1 ? 0 : m_roomTypeIndex + 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneRoomTypeSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
