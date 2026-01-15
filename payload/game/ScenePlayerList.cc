#include "ScenePlayerList.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/OnlineTimer.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>
#include <payload/online/CubeClient.hh>

extern "C" {
#include <stdio.h>
}

ScenePlayerList::ScenePlayerList(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);

    OnlineBackground::Create(m_archive);
    MenuTitleLine::Create(titleLineArchive, heap);

    m_mainScreen.set("PlayerList.blo", 0x0, m_archive);
    for (u32 i = 0; i < m_kartScreens.count(); i++) {
        m_kartScreens[i].set("PlayerListPlayer.blo", 0x0, m_archive);
    }

    for (u32 i = 0; i < m_kartScreens.count(); i++) {
        for (u32 j = 1; j <= 3; j++) {
            m_kartScreens[i].search("RCurs%02u", j)->setHasARTrans(false, true);
            m_kartScreens[i].search("RCurs%02u", j)->setHasARShift(false, true);
            m_kartScreens[i].search("RCurs%02u", j)->setHasARScale(false, true);
        }
    }

    for (u32 i = 0; i < m_kartScreens.count(); i++) {
        m_mainScreen.search("Player%u", i)->appendChild(&m_kartScreens[i]);
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("PlayerList.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);

    OnlineTimer::Create(m_archive);
}

ScenePlayerList::~ScenePlayerList() {}

void ScenePlayerList::init() {
    m_ok = true;

    const CourseManager *courseManager = CourseManager::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 packIndex = SequenceInfo::Instance().m_packIndex;
    m_writeInfo.packCourseCount = courseManager->courseCount(true, raceInfo.isRace(), packIndex);
    m_writeInfo.kartCount = raceInfo.getKartCount();

    s32 prevScene = SequenceApp::Instance()->prevScene();
    if (prevScene != SceneType::CharacterSelect) {
        System::GetDisplay()->startFadeIn(15);
    }

    if ((prevScene == SceneType::CharacterSelect || prevScene == SceneType::MapSelect) &&
            OnlineTimer::Instance()->hasExpired()) {
        m_nextScene = prevScene;
        nextScene();
    } else {
        slideIn();
    }
}

void ScenePlayerList::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);

    OnlineTimer::Instance()->draw(m_graphContext);
}

void ScenePlayerList::calc() {
    CubeClient *client = CubeClient::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;

    m_mainScreen.animation();
    for (u32 i = 0; i < m_kartScreens.count(); i++) {
        m_kartScreens[i].animationMaterials();
    }

    OnlineTimer::Instance()->calc();

    client->writeStatePoll(m_writeInfo);
}

bool ScenePlayerList::clientStateRoom(const ClientStateRoomReadInfo & /* readInfo */) {
    return true;
}

bool ScenePlayerList::clientStateTeam(const ClientStateTeamReadInfo & /* readInfo */) {
    return true;
}

bool ScenePlayerList::clientStatePoll(const ClientStatePollReadInfo &readInfo) {
    m_ok = m_ok && readInfo.ok;
    if (readInfo.ready) {
        OnlineTimer::Instance()->init(0);
    }
    return true;
}

void ScenePlayerList::clientStateError() {
    ErrorViewApp::Call(6);
}

void ScenePlayerList::slideIn() {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    u32 kartCount = raceInfo.getKartCount();
    for (u32 i = 0; i < m_kartScreens.count(); i++) {
        m_mainScreen.search("Player%u", i)->m_isVisible = i < kartCount;
        if (i >= kartCount) {
            continue;
        }
        J2DScreen &screen = m_kartScreens[i];
        u32 colorIndex = onlineInfo.colorIndex(i);
        GXColor color = Race2D::GetPlayerNumberColor(colorIndex);
        for (u32 j = 1; j <= 3; j++) {
            J2DPicture *picture = screen.search("RCurs%02u", j)->downcast<J2DPicture>();
            picture->setWhite(color);
        }
        const Kart &kart = onlineInfo.m_karts[i];
        for (u32 j = 0; j < 2; j++) {
            Array<char, 32> prefix;
            snprintf(prefix.values(), prefix.count(), "PName%" PRIu32, j);
            const Player &player = kart.players[j];
            kart2DCommon->changeUnicodeTexture(player.name.values(), 3, screen, prefix.values());
            for (u32 k = 0; k < 2; k++) {
                J2DPicture *picture = screen.search("P%c%u", "BN"[k], j)->downcast<J2DPicture>();
                picture->m_isVisible = kart.local && j < kart.playerCount;
                if (picture->m_isVisible) {
                    picture->m_cornerColors = Race2D::GetCornerColors(player.index);
                }
                if (k == 1) {
                    Array<char, 32> name;
                    snprintf(name.values(), name.count(), "PlayerNumberSimple_%uP.bti",
                            player.index + 1);
                    picture->changeTexture(name.values(), 0);
                }
            }
        }
        Array<J2DPicture *, 4> pictures;
        for (u32 j = 0; j < pictures.count(); j++) {
            pictures[j] = screen.search("MMR%u", j)->downcast<J2DPicture>();
        }
        s32 mmr = kart.mmr;
        kart2DCommon->changeNumberTexture(mmr, pictures.values(), pictures.count(), false, false);
    }

    MenuTitleLine::Instance()->drop("PlayerList.bti");
    m_mainAnmTransformFrame = 0;
    m_state = &ScenePlayerList::stateSlideIn;
}

void ScenePlayerList::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &ScenePlayerList::stateSlideOut;
}

void ScenePlayerList::idle() {
    m_state = &ScenePlayerList::stateIdle;
}

void ScenePlayerList::nextScene() {
    m_state = &ScenePlayerList::stateNextScene;
}

void ScenePlayerList::stateSlideIn() {
    if (m_mainAnmTransformFrame < 20) {
        m_mainAnmTransformFrame++;
        if (SequenceApp::Instance()->prevScene() != SceneType::CharacterSelect) {
            OnlineTimer *onlineTimer = OnlineTimer::Instance();
            if (m_mainAnmTransformFrame == 1) {
                onlineTimer->init(30);
            }
            if (m_mainAnmTransformFrame <= 15) {
                onlineTimer->setAlpha(m_mainAnmTransformFrame * 17);
            }
        }
    } else {
        idle();
    }
}

void ScenePlayerList::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_nextScene != SceneType::CharacterSelect) {
            if (m_mainAnmTransformFrame <= 15) {
                OnlineTimer::Instance()->setAlpha(m_mainAnmTransformFrame * 17);
            }
        }
    } else {
        nextScene();
    }
}

void ScenePlayerList::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A || OnlineTimer::Instance()->hasExpired()) {
        const OnlineInfo &onlineInfo = OnlineInfo::Instance();
        if (onlineInfo.m_spectating) {
            if (onlineInfo.m_hasCourseSelection) {
                m_nextScene = SceneType::MapSelect;
            } else {
                m_nextScene = SceneType::CoursePoll;
            }
        } else {
            m_nextScene = SceneType::CharacterSelect;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B || !m_ok) {
        m_nextScene = SceneType::Title;
        GameAudio::Main::Instance()->fadeOutAll(15);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL);
        System::GetDisplay()->startFadeOut(15);
        slideOut();
    }
}

void ScenePlayerList::stateNextScene() {
    if (m_nextScene != SceneType::Title && !m_ok) {
        m_nextScene = SceneType::Title;
        GameAudio::Main::Instance()->fadeOutAll(15);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL);
        System::GetDisplay()->startFadeOut(15);
    }

    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
