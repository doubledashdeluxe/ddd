#include "SceneTitle.hh"

#include "game/AppMgr.hh"
#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceApp.hh"
#include "game/RaceInfo.hh"
#include "game/ResMgr.hh"
#include "game/SceneType.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <cube/Console.hh>
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>
#include <payload/CourseManager.hh>
#include <payload/MemoryProtection.hh>
#include <payload/crypto/CubeRandom.hh>
#include <payload/online/Client.hh>
#include <payload/online/CubeServerManager.hh>

SceneTitle::SceneTitle(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap), m_printMemoryCard(heap) {
    m_screen.set("Title.blo", 0x40000, m_archive);

    for (u32 i = 0; i < Entry::Count; i++) {
        for (u32 j = 0; j < 2; j++) {
            m_screen.search("Ttl%c%u", "LR"[j], i + 1)->setHasARTrans(false, false);
            m_screen.search("Ttl%c%u", "LR"[j], i + 1)->setHasARShift(false, false);
            m_screen.search("Ttl%c%u", "LR"[j], i + 1)->setHasARScale(false, false);
            m_screen.search("Ttl%c%u", "LR"[j], i + 1)->setHasARTexCoords(false, false);
        }
    }

    m_startAnmTevRegKey = J2DAnmLoaderDataBase::Load("Title.brk", m_archive);
    m_startAnmTevRegKey->searchUpdateMaterialID(&m_screen);
    m_screen.search("TM2")->setAnimation(m_startAnmTevRegKey);
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Title.bck", m_archive);
        m_screen.search("TtlM%u", i + 1)->setAnimation(m_entryAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_entryAnmTevRegKeys.count(); i++) {
        m_entryAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("Title.brk", m_archive);
        m_entryAnmTevRegKeys[i]->searchUpdateMaterialID(&m_screen);
        m_screen.search("TtlM%u", i + 1)->setAnimation(m_entryAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_entryLineAnmTransforms.count(); i++) {
        m_entryLineAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Title.bck", m_archive);
        for (u32 j = 0; j < 2; j++) {
            m_screen.search("Ttl%c%u", "LR"[j], i + 1)->setAnimation(m_entryLineAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_entryLineAnmTevRegKeys.count(); i++) {
        m_entryLineAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("Title.brk", m_archive);
        m_entryLineAnmTevRegKeys[i]->searchUpdateMaterialID(&m_screen);
        for (u32 j = 0; j < 2; j++) {
            m_screen.search("Ttl%c%u", "LR"[j], i + 1)->setAnimation(m_entryLineAnmTevRegKeys[i]);
        }
    }
}

SceneTitle::~SceneTitle() {}

void SceneTitle::init() {
    Console::Instance()->m_isActive = false;
    MemoryProtection::Finalize();
    SequenceInfo::Instance().m_fromPause = false;
    SequenceInfo::Instance().m_isOnline = false;

    fadeIn();
}

void SceneTitle::draw() {
    m_graphContext->setViewport();

    m_screen.draw(0.0f, 0.0f, m_graphContext);

    m_printMemoryCard.draw();
}

void SceneTitle::calc() {
    (this->*m_state)();

    m_printMemoryCard.calc();

    m_startAnmTevRegKeyFrame = (m_startAnmTevRegKeyFrame + 1) % 240;
    for (u32 i = 0; i < Entry::Count; i++) {
        if (i == m_entryIndex) {
            if (m_entryAnmTransformFrames[i] < 4) {
                m_entryAnmTransformFrames[i]++;
            }
            m_entryAnmTevRegKeyFrames[i] = 1;
            if (m_entryLineAnmTransformFrames[i] < 89) {
                m_entryLineAnmTransformFrames[i]++;
            }
            if (m_entryLineAnmTevRegKeyFrames[i] < 89) {
                m_entryLineAnmTevRegKeyFrames[i]++;
            }
        } else {
            if (m_entryAnmTransformFrames[i] > 0) {
                m_entryAnmTransformFrames[i]--;
            }
            m_entryAnmTevRegKeyFrames[i] = 0;
            m_entryLineAnmTransformFrames[i] = 0;
            m_entryLineAnmTevRegKeyFrames[i] = 0;
        }
    }

    m_startAnmTevRegKey->m_frame = m_startAnmTevRegKeyFrame;
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i]->m_frame = m_entryAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_entryAnmTevRegKeys.count(); i++) {
        m_entryAnmTevRegKeys[i]->m_frame = m_entryAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_entryLineAnmTransforms.count(); i++) {
        m_entryLineAnmTransforms[i]->m_frame = m_entryLineAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_entryLineAnmTevRegKeys.count(); i++) {
        m_entryLineAnmTevRegKeys[i]->m_frame = m_entryLineAnmTevRegKeyFrames[i];
    }

    J2DPicture *picture = m_screen.search("TtlM4")->downcast<J2DPicture>();
    if (m_lanIsEnabled) {
        picture->changeTexture("TTL_Mozi_LANPlay.bti", 0);
    } else {
        picture->changeTexture("TTL_Mozi_OnlinePlay.bti", 0);
    }

    m_screen.animation();
}

void SceneTitle::fadeIn() {
    Client::Instance()->reset();
    CubeServerManager::Instance()->unlock();
    CourseManager::Instance()->unlock();
    m_entryIndex = Entry::Count;
    m_lanIsEnabled = false;
    m_printMemoryCard.reset();
    m_printMemoryCard.init(0x28);
    m_screen.search("TM2")->m_isVisible = true;
    for (u32 i = 0; i < Entry::Count; i++) {
        for (u32 j = 0; j < 3; j++) {
            m_screen.search("Ttl%c%u", "MLR"[j], i + 1)->m_isVisible = false;
        }
    }
    m_startAnmTevRegKeyFrame = 0;
    m_entryAnmTransformFrames.fill(0);
    m_entryLineAnmTransformFrames.fill(0);
    System::GetDisplay()->startFadeIn(15);
    GameAudio::Main *gameAudioMain = GameAudio::Main::Instance();
    if (RaceInfo::Instance().m_waitDemoResult != 1 ||
            gameAudioMain->getPlayingSequenceID() != SoundID::JA_BGM_TITLE) {
        gameAudioMain->startSequenceBgm(SoundID::JA_BGM_TITLE);
    }
    m_state = &SceneTitle::stateFadeIn;
}

void SceneTitle::fadeOut() {
    System::GetDisplay()->setFaderColor(0, 0, 0);
    if (m_nextScene == SceneType::None && s_demoType == 0) {
        System::GetDisplay()->startFadeOut(120);
    } else {
        System::GetDisplay()->startFadeOut(15);
    }
    m_state = &SceneTitle::stateFadeOut;
}

void SceneTitle::idle() {
    m_timeBeforeDemo = 900;
    m_state = &SceneTitle::stateIdle;
}

void SceneTitle::setupCard() {
    m_state = &SceneTitle::stateSetupCard;
}

void SceneTitle::start() {
    m_screen.search("TM2")->m_isVisible = false;
    for (u32 i = 0; i < Entry::Count; i++) {
        for (u32 j = 0; j < 3; j++) {
            m_screen.search("Ttl%c%u", "MLR"[j], i + 1)->m_isVisible = true;
        }
    }
    m_state = &SceneTitle::stateStart;
}

void SceneTitle::startLAN() {
    m_printMemoryCard.init(0x24);
    m_state = &SceneTitle::stateStartLAN;
}

void SceneTitle::nextScene() {
    SequenceInfo::Instance().init();
    OnlineInfo::Instance().reset();
    RaceInfo::Instance().reset();
    m_state = &SceneTitle::stateNextScene;
}

void SceneTitle::nextRace() {
    m_state = &SceneTitle::stateNextRace;
}

void SceneTitle::nextMovie() {
    AppMgr::Request(AppMgr::Request::StartMovieApp | AppMgr::Request::PrepareForMovieApp);
    m_state = &SceneTitle::stateNextMovie;
}

void SceneTitle::stateFadeIn() {
    if (!GameAudio::Main::Instance()->isWaveLoaded(3)) {
        return;
    }

    SequenceApp::Instance()->ready(SceneType::Menu);

    if (System::GetDisplay()->getFaderStatus() != JUTFader::Status::In) {
        return;
    }

    idle();
}

void SceneTitle::stateFadeOut() {
    if (System::GetDisplay()->getFaderStatus() != JUTFader::Status::Out) {
        return;
    }

    if (m_nextScene == SceneType::None) {
        if (s_demoType == 0) {
            nextMovie();
        } else {
            nextRace();
        }
        s_demoType = (s_demoType + 1) % 4;
    } else {
        nextScene();
    }
}

void SceneTitle::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & (PAD_BUTTON_START | PAD_BUTTON_A)) {
        CardAgent::Ask(CardAgent::Command::SetupSystemFile, 0);
        if (CardAgent::IsReady()) {
            m_entryIndex = Entry::Local;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
            start();
        } else {
            setupCard();
        }
    } else if (m_timeBeforeDemo > 0) {
        m_timeBeforeDemo--;
    } else {
        CourseManager *courseManager = CourseManager::Instance();
        if (s_demoType == 0) {
            m_nextScene = SceneType::None;
            GameAudio::Main::Instance()->fadeOutAll(120);
            fadeOut();
        } else {
            if (courseManager->lock()) {
                m_nextScene = SceneType::None;
                RaceInfo &raceInfo = RaceInfo::Instance();
                raceInfo.settingForWaitDemo(s_demoType == 3);
                u32 packIndex = 0;
                u32 courseCount = courseManager->raceCourseCount(packIndex);
                u32 courseIndex = CubeRandom::Instance()->get(courseCount);
                const CourseManager::Course &course =
                        courseManager->raceCourse(packIndex, courseIndex);
                u32 courseOrder = raceInfo.getConsoleCount() < 2 ? 2 : 1;
                ResMgr::LoadExtendedCourseData(&course, courseOrder);
                fadeOut();
            }
        }
    }
}

void SceneTitle::stateSetupCard() {
    if (!CardAgent::IsReady()) {
        return;
    }

    m_entryIndex = Entry::Local;
    CardAgent::Ack(SoundID::JA_SE_TR_DECIDE_LITTLE);
    start();
}

void SceneTitle::stateStart() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & (PAD_BUTTON_START | PAD_BUTTON_A)) {
        if (m_entryIndex == Entry::Remote && m_lanIsEnabled) {
            startLAN();
        } else {
            GameAudio::Main::Instance()->fadeOutAll(15);
            switch (m_entryIndex) {
            case Entry::Local:
                m_nextScene = SceneType::Menu;
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_TITLE_TO_SELECT);
                break;
            case Entry::Records:
                m_nextScene = SceneType::Record;
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
                break;
            case Entry::Options:
                m_nextScene = SceneType::Option;
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
                break;
            case Entry::Remote:
                m_nextScene = SceneType::HowManyPlayers;
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_TITLE_TO_SELECT);
                SequenceInfo::Instance().m_isOnline = true;
                break;
            }
            fadeOut();
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        m_entryIndex = (m_entryIndex - 1) % Entry::Count;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        m_entryIndex = (m_entryIndex + 1) % Entry::Count;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.risingEdge() & PAD_TRIGGER_Z) {
        if (m_entryIndex == Entry::Remote) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_ATTENTION);
            m_lanIsEnabled = !m_lanIsEnabled;
        }
    }
}

void SceneTitle::stateStartLAN() {
    s32 choice = m_printMemoryCard.getFinalChoice();
    if (choice == 0) {
        m_printMemoryCard.ack();
        m_nextScene = SceneType::LanEntry;
        GameAudio::Main::Instance()->fadeOutAll(15);
        fadeOut();
    } else if (choice == 1) {
        m_printMemoryCard.ack();
        m_printMemoryCard.init(0x28);
        start();
    }
}

void SceneTitle::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneTitle::stateNextRace() {
    if (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveID::Course)) {
        return;
    }

    AppMgr::Request(AppMgr::Request::DestroyApp);
    RaceApp::Call();
}

void SceneTitle::stateNextMovie() {}

u32 SceneTitle::s_demoType = 1;
