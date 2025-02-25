#include "SceneTeamSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineTimer.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"

#include <common/Algorithm.hh>
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/crypto/Random.hh>

extern "C" {
#include <stdio.h>
}

SceneTeamSelect::SceneTeamSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *mapSelectArchive = sceneFactory->archive(SceneFactory::ArchiveType::MapSelect);

    m_mainScreen.set("SelectTeams.blo", 0x0, m_archive);
    m_modeScreen.set("SelectMapLayout.blo", 0x1040000, mapSelectArchive);
    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_entryScreens[i].set("SelectTeamsPlayer.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_entryScreens[i].search("Cursor0")->setHasARTrans(false, true);
        m_entryScreens[i].search("Cursor0")->setHasARShift(false, true);
        m_entryScreens[i].search("Cursor0")->setHasARScale(false, true);
    }

    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_mainScreen.search("Player%u", i)->appendChild(&m_entryScreens[i]);
    }

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    J2DScreen &okScreen = m_entryScreens[MaxEntryCount - 1];
    const char *path = "/ok.txt";
    char *ok = reinterpret_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path));
    u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, ok);
    ok[size - 1] = '\0';
    kart2DCommon->changeUnicodeTexture(ok, 6, okScreen, "Ok");
    okScreen.search("L0")->m_isVisible = false;
    okScreen.search("R0")->m_isVisible = false;
    okScreen.search("Team")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectTeams.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_modeAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", mapSelectArchive);
    m_modeScreen.search("NSlMap")->setAnimation(m_modeAnmTransform);
    m_selectAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", mapSelectArchive);
    m_modeScreen.search("NRandom")->setAnimation(m_selectAnmTransform);
    m_modeScreen.search("OK_wb11")->setAnimation(m_selectAnmTransform);
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectTeamsPlayer.bck", m_archive);
        m_entryScreens[i].setAnimation(m_entryAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_entryLeftAnmTransforms.count(); i++) {
        m_entryLeftAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("SelectTeamsPlayer.bck", m_archive);
        m_entryScreens[i].search("L1")->setAnimation(m_entryLeftAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_entryLeftAnmTevRegKeys.count(); i++) {
        m_entryLeftAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("PersonalRoomOption.brk", m_archive);
        m_entryLeftAnmTevRegKeys[i]->searchUpdateMaterialID(&m_entryScreens[i]);
        m_entryScreens[i].search("L1")->setAnimation(m_entryLeftAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_entryRightAnmTransforms.count(); i++) {
        m_entryRightAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("SelectTeamsPlayer.bck", m_archive);
        m_entryScreens[i].search("R1")->setAnimation(m_entryRightAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_entryRightAnmTevRegKeys.count(); i++) {
        m_entryRightAnmTevRegKeys[i] =
                J2DAnmLoaderDataBase::Load("PersonalRoomOption.brk", m_archive);
        m_entryRightAnmTevRegKeys[i]->searchUpdateMaterialID(&m_entryScreens[i]);
        m_entryScreens[i].search("R1")->setAnimation(m_entryRightAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_entryLoopAnmTransforms.count(); i++) {
        m_entryLoopAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("SelectTeamsPlayer.bck", m_archive);
        for (u32 j = 0; j < 2; j++) {
            m_entryScreens[i].search("%c0", "LR"[j])->setAnimation(m_entryLoopAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_entryLoopAnmTextureSRTKeys.count(); i++) {
        m_entryLoopAnmTextureSRTKeys[i] =
                J2DAnmLoaderDataBase::Load("PersonalRoomOption.btk", m_archive);
        m_entryLoopAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_entryScreens[i]);
        for (u32 j = 0; j < 2; j++) {
            m_entryScreens[i].search("%c2", "LR"[j])->setAnimation(m_entryLoopAnmTextureSRTKeys[i]);
        }
    }

    m_selectAnmTransformFrame = 0;
    m_entryAnmTransformFrames.fill(0);
    m_entryLeftAnmTransformFrames.fill(0);
    m_entryLeftAnmTevRegKeyFrames.fill(0);
    m_entryRightAnmTransformFrames.fill(0);
    m_entryRightAnmTevRegKeyFrames.fill(0);
    m_entryLoopAnmTransformFrames.fill(0);
    m_entryLoopAnmTextureSRTKeyFrames.fill(0);

    OnlineTimer::Create(m_archive);
}

SceneTeamSelect::~SceneTeamSelect() {}

void SceneTeamSelect::init() {
    m_playerCount = 8;
    m_teams.fill(0);
    m_entryIndex = 0;
    m_teamCount = 4;

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

void SceneTeamSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_modeScreen.draw(0.0f, 0.0f, m_graphContext);

    OnlineTimer::Instance()->draw(m_graphContext);
}

void SceneTeamSelect::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < m_playerCount; i++) {
        Array<char, 32> path;
        snprintf(path.values(), path.count(), "/teamnames/%u.txt", m_teams[i]);
        char *name =
                reinterpret_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path.values()));
        u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, name);
        name[size - 1] = '\0';
        f32 startX, endX;
        kart2DCommon->changeUnicodeTexture(name, 9, m_entryScreens[i], "Team", &startX, &endX);
        J2DPane *pane = m_entryScreens[i].search("TeamO");
        pane->m_offset.x = -(startX + endX) / 2.0f;
        pane->calcMtx();
        J2DPicture::CornerColors cornerColors = Race2D::GetCornerColors(m_teams[i]);
        for (u32 j = 0; j < 9; j++) {
            J2DPicture *picture = m_entryScreens[i].search("Team%u", j)->downcast<J2DPicture>();
            picture->m_cornerColors = cornerColors;
        }
    }

    for (u32 i = 0; i < MaxEntryCount; i++) {
        if (i == m_entryIndex) {
            if (m_entryAnmTransformFrames[i] < 7) {
                m_entryAnmTransformFrames[i]++;
            }
        } else {
            if (m_entryAnmTransformFrames[i] > 0) {
                m_entryAnmTransformFrames[i]--;
            }
        }
    }
    for (u32 i = 0; i < m_playerCount; i++) {
        if (m_entryLeftAnmTransformFrames[i] > 0) {
            m_entryLeftAnmTransformFrames[i]++;
            if (m_entryLeftAnmTransformFrames[i] == 8) {
                m_entryLeftAnmTransformFrames[i] = 0;
            }
        }
        if (m_entryRightAnmTransformFrames[i] > 0) {
            m_entryRightAnmTransformFrames[i]++;
            if (m_entryRightAnmTransformFrames[i] == 8) {
                m_entryRightAnmTransformFrames[i] = 0;
            }
        }
        if (i == m_entryIndex) {
            if (m_entryLeftAnmTevRegKeyFrames[i] > 1 && m_entryLeftAnmTevRegKeyFrames[i] < 19) {
                m_entryLeftAnmTevRegKeyFrames[i]++;
            } else {
                m_entryLeftAnmTevRegKeyFrames[i] = 1;
            }
            if (m_entryRightAnmTevRegKeyFrames[i] > 1 && m_entryRightAnmTevRegKeyFrames[i] < 19) {
                m_entryRightAnmTevRegKeyFrames[i]++;
            } else {
                m_entryRightAnmTevRegKeyFrames[i] = 1;
            }
            m_entryLoopAnmTransformFrames[i]++;
            if (m_entryLoopAnmTransformFrames[i] >= 30) {
                m_entryLoopAnmTransformFrames[i] = 0;
            }
            m_entryLoopAnmTextureSRTKeyFrames[i]++;
            if (m_entryLoopAnmTextureSRTKeyFrames[i] > 119) {
                m_entryLoopAnmTextureSRTKeyFrames[i] = 0;
            }
        } else {
            m_entryLeftAnmTevRegKeyFrames[i] = 0;
            m_entryRightAnmTevRegKeyFrames[i] = 0;
            m_entryLoopAnmTransformFrames[i] = 0;
            m_entryLoopAnmTextureSRTKeyFrames[i] = 0;
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_modeAnmTransform->m_frame = m_modeAnmTransformFrame;
    m_selectAnmTransform->m_frame = m_selectAnmTransformFrame;
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i]->m_frame = m_entryAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_entryLeftAnmTransforms.count(); i++) {
        m_entryLeftAnmTransforms[i]->m_frame = m_entryLeftAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_entryLeftAnmTevRegKeys.count(); i++) {
        m_entryLeftAnmTevRegKeys[i]->m_frame = m_entryLeftAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_entryRightAnmTransforms.count(); i++) {
        m_entryRightAnmTransforms[i]->m_frame = m_entryRightAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_entryRightAnmTevRegKeys.count(); i++) {
        m_entryRightAnmTevRegKeys[i]->m_frame = m_entryRightAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_entryLoopAnmTransforms.count(); i++) {
        m_entryLoopAnmTransforms[i]->m_frame = m_entryLoopAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_entryLoopAnmTextureSRTKeys.count(); i++) {
        m_entryLoopAnmTextureSRTKeys[i]->m_frame = m_entryLoopAnmTextureSRTKeyFrames[i];
    }

    m_mainScreen.animation();
    m_modeScreen.animation();
    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_entryScreens[i].animationMaterials();
    }

    OnlineTimer::Instance()->calc();
}

void SceneTeamSelect::slideIn() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < MaxPlayerCount; i++) {
        m_mainScreen.search("Player%u", i)->m_isVisible = i < m_playerCount;
        if (i < m_playerCount) {
            J2DScreen &screen = m_entryScreens[i];
            kart2DCommon->changeUnicodeTexture("ABC", 3, screen, "PName0");
            if (i % 2) {
                kart2DCommon->changeUnicodeTexture("   ", 3, screen, "PName1");
            } else {
                kart2DCommon->changeUnicodeTexture("DEF", 3, screen, "PName1");
            }
            for (u32 j = 0; j < 2; j++) {
                for (u32 k = 0; k < 2; k++) {
                    J2DPicture *picture =
                            screen.search("P%c%u", "BN"[k], j)->downcast<J2DPicture>();
                    picture->m_isVisible = j + i * 2 < 3;
                    if (picture->m_isVisible) {
                        picture->m_cornerColors = Race2D::GetCornerColors(j + i * 2);
                    }
                    if (k == 1) {
                        Array<char, 32> name;
                        snprintf(name.values(), name.count(), "PlayerNumberSimple_%uP.bti",
                                j + i * 2 + 1);
                        picture->changeTexture(name.values(), 0);
                    }
                }
            }
        }
    }

    MenuTitleLine::Instance()->drop("SelectTeams.bti");
    m_mainAnmTransformFrame = 0;
    m_state = &SceneTeamSelect::stateSlideIn;
}

void SceneTeamSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneTeamSelect::stateSlideOut;
}

void SceneTeamSelect::idle() {
    m_state = &SceneTeamSelect::stateIdle;
}

void SceneTeamSelect::spin() {
    m_spinFrame = 1;
    m_state = &SceneTeamSelect::stateSpin;
}

void SceneTeamSelect::nextScene() {
    m_state = &SceneTeamSelect::stateNextScene;
}

void SceneTeamSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 22) {
        m_mainAnmTransformFrame++;
        m_modeAnmTransformFrame = Min<u32>(m_mainAnmTransformFrame, 15);
        OnlineTimer *onlineTimer = OnlineTimer::Instance();
        if (m_mainAnmTransformFrame == 1) {
            onlineTimer->init(30 * 60);
        }
        if (m_mainAnmTransformFrame <= 15) {
            onlineTimer->setAlpha(m_mainAnmTransformFrame * 17);
        }
    } else {
        idle();
    }
}

void SceneTeamSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_modeAnmTransformFrame = Min<u32>(m_mainAnmTransformFrame, 15);
        if (m_mainAnmTransformFrame <= 15) {
            OnlineTimer::Instance()->setAlpha(m_mainAnmTransformFrame * 17);
        }
    } else {
        nextScene();
    }
}

void SceneTeamSelect::stateIdle() {
    if (OnlineTimer::Instance()->hasExpired()) {
        if (m_entryIndex + 1 != MaxEntryCount) {
            m_entryIndex = MaxEntryCount - 1;
        }
    }

    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A || OnlineTimer::Instance()->hasExpired()) {
        if (m_entryIndex + 1 == MaxEntryCount) {
            m_nextScene = SceneType::PlayerList;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
            slideOut();
        }
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::PersonalRoom;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_entryIndex >= 1) {
            if (m_entryIndex + 1 == MaxEntryCount) {
                m_entryIndex = m_playerCount - 1;
            } else {
                m_entryIndex--;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_entryIndex + 1 < MaxEntryCount) {
            if (m_entryIndex + 1 == m_playerCount) {
                m_entryIndex = MaxEntryCount - 1;
            } else {
                m_entryIndex++;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        if (m_entryIndex < m_playerCount) {
            m_teams[m_entryIndex] = (m_teams[m_entryIndex] + m_teamCount - 1) % m_teamCount;
            m_entryLeftAnmTransformFrames[m_entryIndex] = 1;
            m_entryLeftAnmTevRegKeyFrames[m_entryIndex] = 2;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        if (m_entryIndex < m_playerCount) {
            m_teams[m_entryIndex] = (m_teams[m_entryIndex] + m_teamCount + 1) % m_teamCount;
            m_entryRightAnmTransformFrames[m_entryIndex] = 1;
            m_entryRightAnmTevRegKeyFrames[m_entryIndex] = 2;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else if ((button.level() & PAD_TRIGGER_R) && (button.level() & PAD_TRIGGER_L)) {
        spin();
    }
}

void SceneTeamSelect::stateSpin() {
    m_spinFrame++;
    bool isSpinning = m_spinFrame < 30;
    if (!isSpinning) {
        const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
        isSpinning = button.level() & PAD_TRIGGER_R && button.level() & PAD_TRIGGER_L;
    }
    if (OnlineTimer::Instance()->hasExpired()) {
        isSpinning = false;
    }
    if (isSpinning) {
        if (m_spinFrame % 5 == 0) {
            for (u32 i = 0; i < m_playerCount; i++) {
                m_teams[i] = i % m_teamCount;
            }
            for (u32 i = 0; i < m_playerCount - 1; i++) {
                u32 j = i + Random::Get(m_playerCount - i);
                Swap(m_teams[i], m_teams[j]);
            }
            m_entryIndex = Random::Get(m_playerCount);
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else {
        m_entryIndex = MaxEntryCount - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        idle();
    }
}

void SceneTeamSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
