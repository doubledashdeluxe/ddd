#include "SceneTeamSelect.hh"

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
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/RoomType.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/crypto/CubeRandom.hh>
#include <payload/online/CubeClient.hh>
#include <portable/Algorithm.hh>

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

    J2DScreen &okScreen = m_entryScreens[MaxEntryCount - 1];
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

    m_mainAnmTransformFrame = 0;
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
    const RaceInfo &raceInfo = RaceInfo::Instance();
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    m_ok = true;
    m_balanced = false;
    m_isSearch = onlineInfo.m_roomType != RoomType::Personal;
    m_isHost = onlineInfo.m_isHost;
    m_canContinue = true;
    m_continuing = false;
    m_kartCount = raceInfo.getKartCount();
    m_teams.fill(0);
    m_entryIndex = 0;
    m_teamCount = onlineInfo.m_teamCount;

    m_writeInfo.isHost = m_isHost;
    m_writeInfo.kartCount = m_kartCount;
    m_writeInfo.teamCount = m_teamCount;
    m_writeInfo.continuing = false;

    J2DPicture *iconPicture = m_modeScreen.search("BtlPict")->downcast<J2DPicture>();
    J2DPicture *namePicture = m_modeScreen.search("SubM")->downcast<J2DPicture>();
    const char *iconTextureName = RaceMode::IconTextureName(raceInfo.m_raceMode);
    iconPicture->changeTexture(iconTextureName, 0);
    const char *nameTextureName = RaceMode::NameTextureName(raceInfo.m_raceMode);
    namePicture->changeTexture(nameTextureName, 0);

    if (m_isSearch) {
        m_nextScene = SceneType::PlayerList;
        nextScene();
    } else {
        slideIn();
    }
}

void SceneTeamSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_modeScreen.draw(0.0f, 0.0f, m_graphContext);

    if (!m_isSearch) {
        OnlineTimer::Instance()->draw(m_graphContext);
    }
}

void SceneTeamSelect::calc() {
    CubeClient *client = CubeClient::Instance();
    client->read(*this);

    calcBalanced();

    (this->*m_state)();

    calcBalanced();
    OnlineInfo::Instance().m_teams = m_teams;

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    J2DScreen &okScreen = m_entryScreens[MaxEntryCount - 1];
    if (m_balanced) {
        const char *path = "/ok.txt";
        char *ok = static_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path));
        u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, ok);
        ok[size - 1] = '\0';
        kart2DCommon->changeUnicodeTexture(ok, 6, okScreen, "Ok");
    } else {
        kart2DCommon->changeUnicodeTexture("...", 6, okScreen, "Ok");
    }
    for (u32 i = 0; i < m_kartCount; i++) {
        Array<char, 32> path;
        snprintf(path.values(), path.count(), "/teamnames/%u.txt", m_teams[i]);
        char *name = static_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path.values()));
        u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, name);
        name[size - 1] = '\0';
        kart2DCommon->changeUnicodeTexture(name, 9, m_entryScreens[i], "Team", true);
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
    for (u32 i = 0; i < m_kartCount; i++) {
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

    if (!m_isSearch) {
        OnlineTimer::Instance()->calc();
    }

    if (m_isHost) {
        m_writeInfo.kartTeams = m_teams;
        m_writeInfo.entryIndex = m_entryIndex;
    }
    client->writeStateTeam(m_writeInfo);
}

bool SceneTeamSelect::clientStateRoom(const ClientStateRoomReadInfo & /* readInfo */) {
    return true;
}

bool SceneTeamSelect::clientStateTeam(const ClientStateTeamReadInfo &readInfo) {
    m_ok = m_ok && readInfo.ok;
    const Optional<ClientStateTeamReadInfo::Info> &info = readInfo.info;
    if (!info) {
        return true;
    }

    m_canContinue = true;
    if (m_isHost && (m_balanced || !info->continuing)) {
        for (u32 i = 0; i < m_kartCount; i++) {
            m_canContinue = m_canContinue && info->kartTeams[i] == m_teams[i];
        }
    } else {
        for (u32 i = 0; i < m_kartCount; i++) {
            u8 team = info->kartTeams[i];
            u32 distance = (m_teamCount + team - m_teams[i]) % m_teamCount;
            if (distance != 0) {
                if (distance <= m_teamCount / 2) {
                    m_entryRightAnmTransformFrames[i] = 1;
                    m_entryRightAnmTevRegKeyFrames[i] = 2;
                } else {
                    m_entryLeftAnmTransformFrames[i] = 1;
                    m_entryLeftAnmTevRegKeyFrames[i] = 2;
                }
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
            }
        }
        m_teams = info->kartTeams;
        if (info->entryIndex != m_entryIndex) {
            if (info->entryIndex < m_kartCount) {
                m_entryIndex = info->entryIndex;
            } else {
                m_entryIndex = MaxEntryCount - 1;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    }
    m_continuing = info->continuing;
    return true;
}

void SceneTeamSelect::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneTeamSelect::slideIn() {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < MaxRoomKartCount; i++) {
        m_mainScreen.search("Player%u", i)->m_isVisible = i < m_kartCount;
        if (i >= m_kartCount) {
            continue;
        }
        J2DScreen &screen = m_entryScreens[i];
        for (u32 j = 0; j < 2; j++) {
            Array<char, 32> prefix;
            snprintf(prefix.values(), prefix.count(), "PName%" PRIu32, j);
            const Kart &kart = onlineInfo.m_karts[i];
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
    }

    MenuTitleLine::Instance()->drop("SelectTeams.bti");
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
            onlineTimer->init(30);
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
    if ((m_isHost && button.risingEdge() & PAD_BUTTON_A && m_entryIndex + 1 == MaxEntryCount) ||
            OnlineTimer::Instance()->hasExpired() || (m_balanced && m_continuing)) {
        if (m_isHost) {
            m_writeInfo.continuing = true;
        }
        m_nextScene = SceneType::PlayerList;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B || !m_ok || (!m_balanced && m_continuing)) {
        m_nextScene = SceneType::RoomTypeSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_entryIndex >= 1) {
            if (m_entryIndex + 1 == MaxEntryCount) {
                m_entryIndex = m_kartCount - 1;
            } else {
                m_entryIndex--;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_entryIndex + 1 < MaxEntryCount) {
            if (m_entryIndex + 1 == m_kartCount) {
                m_entryIndex = MaxEntryCount - 1;
            } else {
                m_entryIndex++;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        if (m_entryIndex < m_kartCount) {
            m_teams[m_entryIndex] = (m_teams[m_entryIndex] + m_teamCount - 1) % m_teamCount;
            m_entryLeftAnmTransformFrames[m_entryIndex] = 1;
            m_entryLeftAnmTevRegKeyFrames[m_entryIndex] = 2;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        if (m_entryIndex < m_kartCount) {
            m_teams[m_entryIndex] = (m_teams[m_entryIndex] + m_teamCount + 1) % m_teamCount;
            m_entryRightAnmTransformFrames[m_entryIndex] = 1;
            m_entryRightAnmTevRegKeyFrames[m_entryIndex] = 2;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else if (m_isHost && (button.level() & PAD_TRIGGER_R) && (button.level() & PAD_TRIGGER_L)) {
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
            for (u32 i = 0; i < m_kartCount; i++) {
                m_teams[i] = i % m_teamCount;
            }
            CubeRandom *random = CubeRandom::Instance();
            for (u32 i = 0; i < m_kartCount - 1; i++) {
                u32 j = i + random->get(m_kartCount - i);
                Swap(m_teams[i], m_teams[j]);
            }
            m_entryIndex = random->get(m_kartCount);
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else {
        m_entryIndex = MaxEntryCount - 1;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        idle();
    }
}

void SceneTeamSelect::stateNextScene() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (m_nextScene == SceneType::PlayerList) {
        if (button.risingEdge() & PAD_BUTTON_B || !m_ok || (!m_balanced && m_continuing)) {
            if (m_isSearch) {
                m_nextScene = SceneType::FormatSelect;
            } else {
                m_nextScene = SceneType::RoomTypeSelect;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        }
    }

    if (m_nextScene == SceneType::PlayerList) {
        if (!m_canContinue || !m_continuing) {
            return;
        }
    }

    if (m_nextScene == SceneType::PlayerList) {
        if (!System::GetDisplay()->ensureOut(15)) {
            return;
        }
    } else {
        if (!System::GetDisplay()->ensureIn(15)) {
            return;
        }
    }

    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneTeamSelect::calcBalanced() {
    Array<u8, MaxTeamCount> teamSizes(0);
    for (u32 i = 0; i < m_kartCount; i++) {
        teamSizes[m_teams[i]]++;
    }
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    m_balanced = true;
    for (u32 i = 0; i < teamSizes.count(); i++) {
        m_balanced = m_balanced && teamSizes[i] <= onlineInfo.m_maxTeamSize;
    }
}
