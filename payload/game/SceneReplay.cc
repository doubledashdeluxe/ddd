#include "SceneReplay.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/Modes.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

extern "C" {
#include <dolphin/OSTime.h>
}
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>
#include <payload/Lock.hh>
#include <payload/online/CubeClient.hh>
#include <payload/online/CubeReplayManager.hh>
#include <portable/Algorithm.hh>
#include <portable/UTF8.hh>

SceneReplay::SceneReplay(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap), m_heap(heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);

    OnlineBackground::Create(lanEntryArchive);
    MenuTitleLine::Create(titleLineArchive, heap);

    m_mainScreen.set("GDIndexLayout.blo", 0x20000, m_archive);
    m_downloadScreen.set("EnterRoomCode.blo", 0x1040000, lanEntryArchive);
    for (u32 i = 0; i < m_replayScreens.count(); i++) {
        m_replayScreens[i].set("Replay.blo", 0x20000, m_archive);
    }
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        for (u32 j = 0; j < m_playerScreens[i].count(); j++) {
            m_playerScreens[i][j].set("ReplayPlayer.blo", 0x1040000, m_archive);
        }
    }

    for (u32 i = 0; i < m_replayScreens.count(); i++) {
        m_replayScreens[i].search("GDCurs")->setHasARTrans(false, true);
        m_replayScreens[i].search("GDCurs")->setHasARShift(false, true);
        m_replayScreens[i].search("GDCurs")->setHasARScale(false, true);
    }

    for (u32 i = 0; i < m_replayScreens.count(); i++) {
        m_mainScreen.search("Num%02u", i)->appendChild(&m_replayScreens[i]);
    }
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        for (u32 j = 0; j < m_playerScreens[i].count(); j++) {
            m_replayScreens[i].search("Player%u", j)->appendChild(&m_playerScreens[i][j]);
        }
    }
    m_mainScreen.search("NSaveGD")->m_isVisible = false;
    J2DPicture *downloadPicture = m_downloadScreen.search("Ran3")->downcast<J2DPicture>();
    downloadPicture->changeTexture("Download.bti", 0);

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectPackLayout.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("SelectPackLayout.bck", m_archive);
    for (u32 i = 0; i < 2; i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAnimation(m_arrowAnmTransform);
    }
    m_downloadAnmTransform = J2DAnmLoaderDataBase::Load("EnterRoomCode.bck", lanEntryArchive);
    m_downloadScreen.search("NSlMap")->setAnimation(m_downloadAnmTransform);
    m_selectAnmTransform = J2DAnmLoaderDataBase::Load("EnterRoomCode.bck", lanEntryArchive);
    m_downloadScreen.search("NRandom")->setAnimation(m_selectAnmTransform);
    m_downloadScreen.search("OK_wb11")->setAnimation(m_selectAnmTransform);
    for (u32 i = 0; i < m_replayAnmTransforms.count(); i++) {
        m_replayAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_replayScreens[i].setAnimation(m_replayAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_playerAnmTransforms.count(); i++) {
        for (u32 j = 0; j < m_playerAnmTransforms[i].count(); j++) {
            m_playerAnmTransforms[i][j] = J2DAnmLoaderDataBase::Load("ReplayPlayer.bck", m_archive);
            m_playerScreens[i][j].setAnimation(m_playerAnmTransforms[i][j]);
        }
    }

    m_mainAnmTransformFrame = 0;
    m_arrowAnmTransformFrame = 0;
    m_downloadAnmTransformFrame = 0;
    m_replayAnmTransformFrames.fill(0);
    m_playerAnmTransformFrames.fill(1);
    m_arrowAlphas.fill(0);
    m_replayAlphas.fill(0);
}

SceneReplay::~SceneReplay() {
    if (m_loadStack) {
        m_loading = false;
        OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
        OSJoinThread(&m_loadThread, nullptr);
    }
}

void SceneReplay::init() {
    m_replayCount = 0;

    System::GetDisplay()->startFadeIn(15);

    m_selectAnmTransformFrame = 10;

    if (CubeReplayManager::Instance()->lock() && CourseManager::Instance()->lock()) {
        slideIn();
    } else {
        wait();
    }
}

void SceneReplay::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_downloadScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneReplay::calc() {
    (this->*m_state)();

    for (u32 i = 0; i < m_replayScreens.count(); i++) {
        Lock<Mutex> lock(m_mutex);
        u32 replayIndex = m_rowIndex + i;
        if (replayIndex >= m_replayCount) {
            break;
        }
        u32 j;
        for (j = 0; j < m_nextReplayIndices.count(); j++) {
            if (m_nextReplayIndices[j] == replayIndex) {
                break;
            }
        }
        if (j == m_nextReplayIndices.count()) {
            continue;
        }
        ResTIMG *logo = nullptr;
        for (u32 j = 0; j < m_currReplayIndices.count(); j++) {
            if (m_currReplayIndices[j] == replayIndex) {
                logo = m_logos[j].get();
                break;
            }
        }
        if (logo) {
            J2DPicture *picture = m_replayScreens[i].search("Logo")->downcast<J2DPicture>();
            picture->m_isVisible = true;
            picture->changeTexture(logo, 0);
        }
    }

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 35;
    for (u32 i = 0; i < 6; i++) {
        u32 replayIndex = m_rowIndex + i;
        if (replayIndex == m_replayIndex) {
            if (m_replayAnmTransformFrames[i] < 7) {
                m_replayAnmTransformFrames[i]++;
            }
        } else {
            if (m_replayAnmTransformFrames[i] > 0) {
                m_replayAnmTransformFrames[i]--;
            }
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_arrowAnmTransform->m_frame = m_arrowAnmTransformFrame;
    m_downloadAnmTransform->m_frame = m_downloadAnmTransformFrame;
    m_selectAnmTransform->m_frame = m_selectAnmTransformFrame;
    for (u32 i = 0; i < m_replayAnmTransforms.count(); i++) {
        m_replayAnmTransforms[i]->m_frame = m_replayAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_playerAnmTransforms.count(); i++) {
        for (u32 j = 0; j < m_playerAnmTransforms[i].count(); j++) {
            m_playerAnmTransforms[i][j]->m_frame = m_playerAnmTransformFrames[i][j];
        }
    }

    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }
    for (u32 i = 0; i < m_replayAlphas.count(); i++) {
        m_replayScreens[i].search("GDCurs")->setAlpha(m_replayAlphas[i]);
        m_replayScreens[i].search("GDCurs1")->setAlpha(m_replayAlphas[i]);
        m_replayScreens[i].search("Mode")->setAlpha(m_replayAlphas[i]);
        m_replayScreens[i].search("Logo")->setAlpha(m_replayAlphas[i]);
        for (u32 j = 0; j < 23; j++) {
            m_replayScreens[i].search("Time%u", j)->setAlpha(m_replayAlphas[i]);
        }
        for (u32 j = 0; j < m_playerScreens[i].count(); j++) {
            J2DScreen &screen = m_playerScreens[i][j];
            for (u32 k = 0; k < 2; k++) {
                for (u32 l = 0; l < 3; l++) {
                    screen.search("PName%u%u", k, l)->setAlpha(m_replayAlphas[i]);
                }
            }
        }
    }

    m_mainScreen.animation();
    m_downloadScreen.animation();
    for (u32 i = 0; i < m_replayScreens.count(); i++) {
        m_replayScreens[i].animationMaterials();
    }
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        for (u32 j = 0; j < m_playerScreens[i].count(); j++) {
            m_playerScreens[i][j].animationMaterials();
        }
    }
}

void SceneReplay::wait() {
    m_state = &SceneReplay::stateWait;
}

void SceneReplay::slideIn() {
    CubeReplayManager *replayManager = CubeReplayManager::Instance();
    replayManager->filterAndSort();
    m_replayCount = replayManager->replayCount();
    m_replayIndex = 0;
    m_rowIndex = m_replayIndex;
    m_rowIndex = Min(m_rowIndex, m_replayCount - Min<u32>(m_replayCount, 5));
    m_clientIndex = 0;

    MenuTitleLine::Instance()->drop("Replays.bti");
    for (u32 i = 0; i < m_replayAlphas.count(); i++) {
        u32 replayIndex = m_rowIndex + i;
        if (i < 5 && replayIndex < m_replayCount) {
            m_replayAlphas[i] = 255;
        } else {
            m_replayAlphas[i] = 0;
        }
    }
    m_arrowAlphas.fill(0);
    m_loading = true;
    m_currReplayIndices.fill(UINT32_MAX);
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    u32 stackSize = 64 * 1024;
    m_loadStack.reset(new (m_heap, 0x8) u8[stackSize]);
    OSCreateThread(&m_loadThread, Load, this, m_loadStack.get() + stackSize, stackSize, 25, 0);
    OSResumeThread(&m_loadThread);
    refreshReplays(false);
    m_state = &SceneReplay::stateSlideIn;
}

void SceneReplay::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_mainAnmTransformFrame = 30;
    m_loading = false;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
    m_state = &SceneReplay::stateSlideOut;
}

void SceneReplay::idle() {
    m_state = &SceneReplay::stateIdle;
}

void SceneReplay::scrollUp() {
    m_replayIndex--;
    m_rowIndex--;
    m_mainAnmTransformFrame = 46;
    m_replayAnmTransformFrames.rotateRight(1);
    m_replayAlphas.rotateRight(1);
    refreshReplays(false);
    m_state = &SceneReplay::stateScrollUp;
}

void SceneReplay::scrollDown() {
    m_replayIndex++;
    m_mainAnmTransformFrame = 40;
    m_state = &SceneReplay::stateScrollDown;
}

void SceneReplay::selectIn() {
    m_clientCount = CubeReplayManager::Instance()->replay(m_replayIndex).clients.count();
    refreshReplays(true);
    m_selectAnmTransformFrame = 11;
    m_state = &SceneReplay::stateSelectIn;
}

void SceneReplay::selectOut() {
    refreshReplays(false);
    m_selectAnmTransformFrame = 18;
    m_state = &SceneReplay::stateSelectOut;
}

void SceneReplay::select() {
    m_state = &SceneReplay::stateSelect;
}

void SceneReplay::nextScene() {
    for (u32 i = 0; i < m_logos.count(); i++) {
        m_logos[i].reset();
    }
    refreshReplays(false);
    m_state = &SceneReplay::stateNextScene;
}

void SceneReplay::stateWait() {
    if (CourseManager::Instance()->lock()) {
        slideIn();
    }
}

void SceneReplay::stateSlideIn() {
    if (m_mainAnmTransformFrame < 30) {
        m_mainAnmTransformFrame++;
        m_downloadAnmTransformFrame = Min<u32>(m_mainAnmTransformFrame, 15);
        if (m_mainAnmTransformFrame > 25) {
            showArrows(0);
        }
    } else {
        idle();
    }
}

void SceneReplay::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_downloadAnmTransformFrame = Min<u32>(m_mainAnmTransformFrame, 15);
        hideArrows();
    } else {
        if (OSIsThreadTerminated(&m_loadThread)) {
            OSDetachThread(&m_loadThread);
            m_loadStack.reset();
            nextScene();
        }
    }
}

void SceneReplay::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        if (m_replayCount != 0) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
            selectIn();
        }
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::Title;
        GameAudio::Main::Instance()->fadeOutAll(15);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL);
        System::GetDisplay()->startFadeOut(15);
        slideOut();
    } else if (button.risingEdge() & PAD_TRIGGER_Z) {
        m_nextScene = SceneType::Title;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_replayIndex >= 1) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_replayIndex == m_rowIndex) {
                scrollUp();
            } else {
                m_replayIndex--;
            }
            m_clientIndex = 0;
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_replayIndex + 1 < m_replayCount) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_replayIndex == m_rowIndex + 4) {
                scrollDown();
            } else {
                m_replayIndex++;
            }
            m_clientIndex = 0;
        }
    }
}

void SceneReplay::stateScrollUp() {
    m_mainAnmTransformFrame--;
    showReplays(0);
    showArrows(0);
    if (m_mainAnmTransformFrame == 39) {
        idle();
    }
}

void SceneReplay::stateScrollDown() {
    m_mainAnmTransformFrame++;
    showReplays(1);
    showArrows(1);
    if (m_mainAnmTransformFrame == 47) {
        m_rowIndex++;
        m_mainAnmTransformFrame = 39;
        m_replayAnmTransformFrames.rotateLeft(1);
        m_replayAlphas.rotateLeft(1);
        refreshReplays(false);
        idle();
    }
}

void SceneReplay::stateSelectIn() {
    m_selectAnmTransformFrame++;
    hideArrows();
    if (m_selectAnmTransformFrame == 19) {
        select();
    }
}

void SceneReplay::stateSelectOut() {
    m_selectAnmTransformFrame--;
    showArrows(0);
    if (m_selectAnmTransformFrame == 10) {
        idle();
    }
}

void SceneReplay::stateSelect() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::PersonalRoom;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
        const CubeReplayManager *replayManager = CubeReplayManager::Instance();
        const ReplayManager::Replay &replay = replayManager->replay(m_replayIndex);
        const ReplayManager::Client *client = nullptr;
        if (m_clientIndex < replay.clients.count()) {
            client = &replay.clients[m_clientIndex];
        }
        SequenceInfo &sequenceInfo = SequenceInfo::Instance();
        sequenceInfo.m_padCount = client ? client->players.count() : 1;
        sequenceInfo.m_statusCount = client ? client->teams.count() : 1;
        OnlineInfo &onlineInfo = OnlineInfo::Instance();
        onlineInfo.m_isReplay = true;
        if (client) {
            for (u32 i = 0; i < client->players.count(); i++) {
                onlineInfo.m_names[i] = client->players[i].name;
            }
        } else {
            onlineInfo.m_names[0] = "   ";
        }
        onlineInfo.setLocalKarts();
        onlineInfo.m_roomType = replay.roomType;
        onlineInfo.m_modeIndex = replay.modeIndex;
        onlineInfo.m_format = replay.format;
        onlineInfo.m_isHost = replay.roomType == RoomType::Personal && m_clientIndex == 0;
        onlineInfo.m_roomCounter = 0;
        onlineInfo.m_roomCode = replay.roomCode;
        RaceInfo::Instance().m_raceMode = Modes[replay.modeIndex];
        CubeClient::Instance()->setReplay(replay, m_clientIndex);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        selectOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        m_clientIndex = m_clientIndex == 0 ? m_clientCount : m_clientIndex - 1;
        refreshReplays(true);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        m_clientIndex = m_clientIndex == m_clientCount ? 0 : m_clientIndex + 1;
        refreshReplays(true);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneReplay::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneReplay::refreshReplays(bool playerColors) {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    const CubeReplayManager *replayManager = CubeReplayManager::Instance();
    for (u32 i = 0; i < m_replayScreens.count(); i++) {
        u32 replayIndex = m_rowIndex + i;
        if (replayIndex >= m_replayCount) {
            break;
        }
        const ReplayManager::Replay &replay = replayManager->replay(replayIndex);
        J2DScreen &screen = m_replayScreens[i];
        J2DPicture *modePicture = m_replayScreens[i].search("Mode")->downcast<J2DPicture>();
        modePicture->changeTexture(ModeIconTextureNames[replay.modeIndex], 0);
        J2DPicture *logoPicture = m_replayScreens[i].search("Logo")->downcast<J2DPicture>();
        logoPicture->m_isVisible = false;
        logoPicture->changeTexture("SelCourse_Pict_Box1.bti", 0);
        OSCalendarTime time;
        OSTicksToCalendarTime(replay.time, &time);
        char timeText[24];
        snprintf(timeText, Count(timeText), "%04d-%02d-%02d %02d:%02d:%02d UTC", time.year,
                time.mon + 1, time.mday, time.hour, time.min, time.sec);
        kart2DCommon->changeUnicodeTexture(timeText, 23, screen, "Time", false);
        const char *names[MaxRoomKartCount][4] = {};
        u32 colorIndices[MaxRoomKartCount][2];
        for (u32 j = 0, k = 0; j < replay.clients.count(); j++) {
            const ReplayManager::Client &client = replay.clients[j];
            u32 tandemCount = client.players.count() - client.teams.count();
            for (u32 l = 0; l < client.teams.count(); k++, l++) {
                if (l < tandemCount) {
                    names[k][0] = client.players[l / 2 + 0].name.values();
                    names[k][1] = client.players[l / 2 + 1].name.values();
                } else {
                    names[k][0] = client.players[l + tandemCount].name.values();
                }
                if (playerColors) {
                    if (replayIndex == m_replayIndex && j == m_clientIndex) {
                        if (l < tandemCount) {
                            colorIndices[k][0] = l / 2 + 0;
                            colorIndices[k][1] = l / 2 + 1;
                        } else {
                            colorIndices[k][0] = l + tandemCount;
                            colorIndices[k][1] = l + tandemCount;
                        }
                    } else {
                        colorIndices[k][0] = 8;
                        colorIndices[k][1] = 8;
                    }
                } else {
                    colorIndices[k][0] = client.teams[l];
                    colorIndices[k][1] = client.teams[l];
                }
            }
        }
        for (u32 j = 0; j < m_playerScreens[i].count(); j++) {
            J2DScreen &screen = m_playerScreens[i][j];
            for (u32 k = 0; k < 2; k++) {
                const char *name = names[j][k] ? names[j][k] : "   ";
                char prefix[32];
                snprintf(prefix, Count(prefix), "PName%" PRIu32, k);
                kart2DCommon->changeUnicodeTexture(name, 3, screen, prefix);
            }
            if (j >= replay.kartCount) {
                continue;
            }
            for (u32 k = 0; k < 2; k++) {
                u32 colorIndex = colorIndices[j][k];
                J2DPicture::CornerColors cornerColors;
                if (colorIndex < 8) {
                    cornerColors = Race2D::GetCornerColors(colorIndex);
                } else {
                    cornerColors.topLeft = (GXColor){255, 255, 255, 255};
                    cornerColors.topRight = (GXColor){85, 85, 85, 255};
                    cornerColors.bottomLeft = (GXColor){85, 85, 85, 255};
                    cornerColors.bottomRight = (GXColor){255, 255, 255, 255};
                }
                for (u32 l = 0; l < 3; l++) {
                    J2DPicture *picture = screen.search("PName%u%u", k, l)->downcast<J2DPicture>();
                    picture->m_cornerColors = cornerColors;
                }
            }
            m_playerAnmTransformFrames[i][j] = names[j][1] ? 2 : 1;
        }
    }

    Array<u32, 12> nextReplayIndices;
    u32 rowIndex = Max<u32>(m_rowIndex, 3) - 3;
    for (u32 i = 0; i < nextReplayIndices.count(); i++) {
        nextReplayIndices[i] = rowIndex + i;
    }

    Lock<Mutex> m_lock(m_mutex);
    m_nextReplayIndices = nextReplayIndices;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void SceneReplay::showReplays(s32 rowOffset) {
    for (u32 i = 0; i < m_replayAlphas.count(); i++) {
        u32 replayIndex = m_rowIndex + i;
        if (static_cast<s32>(i) >= rowOffset && static_cast<s32>(i) < 5 + rowOffset &&
                replayIndex < m_replayCount) {
            if (m_replayAlphas[i] < 255) {
                m_replayAlphas[i] += 51;
            }
        } else {
            if (m_replayAlphas[i] > 0) {
                m_replayAlphas[i] -= 51;
            }
        }
    }
}

void SceneReplay::showArrows(s32 rowOffset) {
    if (m_rowIndex + rowOffset > 0) {
        if (m_arrowAlphas[0] < 255) {
            m_arrowAlphas[0] += 51;
        }
    } else {
        if (m_arrowAlphas[0] > 0) {
            m_arrowAlphas[0] -= 51;
        }
    }
    if (m_rowIndex + rowOffset + 5 < m_replayCount) {
        if (m_arrowAlphas[1] < 255) {
            m_arrowAlphas[1] += 51;
        }
    } else {
        if (m_arrowAlphas[1] > 0) {
            m_arrowAlphas[1] -= 51;
        }
    }
}

void SceneReplay::hideArrows() {
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        if (m_arrowAlphas[i] > 0) {
            m_arrowAlphas[i] -= 51;
        }
    }
}

void *SceneReplay::load() {
    while (m_loading) {
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);

        while (m_loading) {
            Array<u32, 12> nextReplayIndices;
            {
                Lock<Mutex> lock(m_mutex);
                nextReplayIndices = m_nextReplayIndices;
            }

            if (load(nextReplayIndices)) {
                break;
            }
        }
    }
    return nullptr;
}

bool SceneReplay::load(const Array<u32, 12> &nextReplayIndices) {
    const CubeReplayManager *replayManager = CubeReplayManager::Instance();
    const CourseManager *courseManager = CourseManager::Instance();
    for (u32 i = 0; i < nextReplayIndices.count(); i++) {
        u32 replayIndex = nextReplayIndices[i];
        if (replayIndex >= m_replayCount) {
            continue;
        }
        const ReplayManager::Replay &replay = replayManager->replay(replayIndex);
        const CourseManager::Course &course = courseManager->courseByHash(true, replay.isRace,
                replay.packIndex, replay.courseIndex);
        UniquePtr<ResTIMG> &logo = findLogo(nextReplayIndices, replayIndex);
        if (!logo.get()) {
            void *texture = course.loadLogo(m_heap);
            {
                Lock<Mutex> lock(m_mutex);
                logo.reset(static_cast<ResTIMG *>(texture));
            }
            return false;
        }
    }
    return true;
}

UniquePtr<ResTIMG> &SceneReplay::findLogo(const Array<u32, 12> &nextReplayIndices,
        u32 replayIndex) {
    for (u32 i = 0; i < m_currReplayIndices.count(); i++) {
        if (m_currReplayIndices[i] == replayIndex) {
            return m_logos[i];
        }
    }
    u32 i;
    for (i = 0; i < m_currReplayIndices.count(); i++) {
        u32 j;
        for (j = 0; j < nextReplayIndices.count(); j++) {
            if (nextReplayIndices[j] == m_currReplayIndices[i]) {
                break;
            }
        }
        if (j == nextReplayIndices.count()) {
            break;
        }
    }
    assert(i < m_currReplayIndices.count());
    Lock<Mutex> lock(m_mutex);
    m_logos[i].reset();
    m_currReplayIndices[i] = replayIndex;
    return m_logos[i];
}

void *SceneReplay::Load(void *param) {
    return static_cast<SceneReplay *>(param)->load();
}
