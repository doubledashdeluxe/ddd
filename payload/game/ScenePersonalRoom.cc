#include "ScenePersonalRoom.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/Modes.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/RoomOption.hh"
#include "game/RoomType.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>
#include <payload/crypto/CubeRandom.hh>
#include <payload/online/CubeClient.hh>
#include <portable/Align.hh>

extern "C" {
#include <stdio.h>
}

ScenePersonalRoom::ScenePersonalRoom(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_mainScreen.set("PersonalRoom.blo", 0x20000, m_archive);
    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_charScreens[i].set("PersonalRoomChar.blo", 0x20000, m_archive);
    }
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        m_playerScreens[i].set("PersonalRoomPlayer.blo", 0x0, m_archive);
    }
    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_entryScreens[i].set("PersonalRoomOption.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_entryScreens[i].search("Cursor0")->setHasARTrans(false, true);
        m_entryScreens[i].search("Cursor0")->setHasARShift(false, true);
        m_entryScreens[i].search("Cursor0")->setHasARScale(false, true);
    }

    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_mainScreen.search("Char%u", i)->appendChild(&m_charScreens[i]);
    }
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        m_mainScreen.search("Player%u", i)->appendChild(&m_playerScreens[i]);
    }
    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_mainScreen.search("Option%u", i)->appendChild(&m_entryScreens[i]);
    }
    J2DScreen &okScreen = m_entryScreens[MaxEntryCount - 1];
    okScreen.search("L0")->m_isVisible = false;
    okScreen.search("R0")->m_isVisible = false;
    okScreen.search("Value")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("PersonalRoom.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    for (u32 i = 0; i < m_switchAnmTexPatterns.count(); i++) {
        m_switchAnmTexPatterns[i] = J2DAnmLoaderDataBase::Load("PersonalRoom.btp", m_archive);
        m_switchAnmTexPatterns[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("Switch%uM", i)->setAnimation(m_switchAnmTexPatterns[i]);
    }
    for (u32 i = 0; i < m_switchAnmTransforms.count(); i++) {
        m_switchAnmTransforms[i] = J2DAnmLoaderDataBase::Load("PersonalRoom.bck", m_archive);
        m_mainScreen.search("Switch%u", i)->setAnimation(m_switchAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_charAnmTextureSRTKeys.count(); i++) {
        m_charAnmTextureSRTKeys[i] = J2DAnmLoaderDataBase::Load("PersonalRoomChar.btk", m_archive);
        m_charAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_charScreens[i]);
        m_charScreens[i].search("Char")->setAnimation(m_charAnmTextureSRTKeys[i]);
    }
    for (u32 i = 0; i < m_charAnmTexPatterns.count(); i++) {
        m_charAnmTexPatterns[i] = J2DAnmLoaderDataBase::Load("PersonalRoomChar.btp", m_archive);
        m_charAnmTexPatterns[i]->searchUpdateMaterialID(&m_charScreens[i]);
        m_charScreens[i].search("Char")->setAnimation(m_charAnmTexPatterns[i]);
    }
    for (u32 i = 0; i < m_playerAnmTransforms.count(); i++) {
        m_playerAnmTransforms[i] = J2DAnmLoaderDataBase::Load("PersonalRoomPlayer.bck", m_archive);
        m_playerScreens[i].setAnimation(m_playerAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_playerNameAnmTransforms.count(); i++) {
        m_playerNameAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("PersonalRoomPlayer.bck", m_archive);
        for (u32 j = 0; j < 3; j++) {
            m_playerScreens[i].search("PName0%u", j)->setAnimation(m_playerNameAnmTransforms[i]);
        }
    }
    m_playerCircleAnmTransform = J2DAnmLoaderDataBase::Load("PersonalRoomPlayer.bck", m_archive);
    for (u32 i = 0; i < MaxRoomKartCount; i++) {
        m_playerScreens[i].search("PCircle")->setAnimation(m_playerCircleAnmTransform);
    }
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i] = J2DAnmLoaderDataBase::Load("PersonalRoomOption.bck", m_archive);
        m_entryScreens[i].setAnimation(m_entryAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_entryLeftAnmTransforms.count(); i++) {
        m_entryLeftAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("PersonalRoomOption.bck", m_archive);
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
                J2DAnmLoaderDataBase::Load("PersonalRoomOption.bck", m_archive);
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
                J2DAnmLoaderDataBase::Load("PersonalRoomOption.bck", m_archive);
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
    m_switchAnmTransformFrames.fill(16);
    m_switchAnmTexPatternFrames.fill(0);
    m_charAnmTextureSRTKeyFrames.fill(9);
    m_charAnmTexPatternFrames.fill(9);
    m_playerAnmTransformFrames.fill(0);
    m_playerNameAnmTransformFrames.fill(0);
    m_playerCircleAnmTransformFrame = 0;
    m_entryAnmTransformFrames.fill(0);
    m_entryLeftAnmTransformFrames.fill(0);
    m_entryLeftAnmTevRegKeyFrames.fill(0);
    m_entryRightAnmTransformFrames.fill(0);
    m_entryRightAnmTevRegKeyFrames.fill(0);
    m_entryLoopAnmTransformFrames.fill(0);
    m_entryLoopAnmTextureSRTKeyFrames.fill(0);
}

ScenePersonalRoom::~ScenePersonalRoom() {}

void ScenePersonalRoom::init() {
    OnlineInfo &onlineInfo = OnlineInfo::Instance();
    m_isSearch = onlineInfo.m_roomType == RoomType::Worldwide;
    m_isHost = onlineInfo.m_isHost;
    m_canContinue = true;
    m_continuing = false;
    m_charCount = 0;
    m_revealCode = false;
    m_kartCount = 0;
    m_optionCount = 0;
    m_entryIndex = 0;

    m_writeInfo.isSearch = m_isSearch;
    if (m_isSearch || m_isHost) {
        m_isRace = RaceInfo::Instance().isRace();
        m_writeInfo.modeIndex = onlineInfo.m_modeIndex;
        m_writeInfo.isRace = m_isRace;
        u32 packIndex = SequenceInfo::Instance().m_packIndex;
        const CourseManager *courseManager = CourseManager::Instance();
        const CourseManager::Pack &pack = courseManager->pack(true, m_isRace, packIndex);
        m_writeInfo.packCourseCount = pack.courseIndices().count();
        m_writeInfo.packHash = pack.hash();
    }
    m_writeInfo.isHost = m_isHost;
    m_writeInfo.roomCounter = onlineInfo.m_roomCounter++;
    if (!m_isHost) {
        m_writeInfo.roomCode = onlineInfo.m_roomCode;
    }
    m_writeInfo.spectatingCounter = 0;
    m_writeInfo.spectating = false;
    if (m_isSearch) {
        u8 format = onlineInfo.m_format;
        m_writeInfo.options.format = format;
        m_writeInfo.options.matchCount = format == RoomOptionFormat::Duel ? 5 : 0;
    }
    m_writeInfo.continuing = false;

    wait();
}

void ScenePersonalRoom::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
}

void ScenePersonalRoom::calc() {
    CubeClient *client = CubeClient::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    J2DScreen &okScreen = m_entryScreens[MaxEntryCount - 1];
    if (m_kartCount >= 2) {
        const char *path = "/ok.txt";
        char *ok = static_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path));
        u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, ok);
        ok[size - 1] = '\0';
        kart2DCommon->changeUnicodeTexture(ok, 20, okScreen, "Name");
    } else {
        kart2DCommon->changeUnicodeTexture("...", 20, okScreen, "Name");
    }
    for (u32 i = 0; i < m_optionCount; i++) {
        Array<char, 32> path;
        snprintf(path.values(), path.count(), "/valuenames/%u/%u.txt", m_options[i], m_values[i]);
        char *name = static_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path.values()));
        if (name) {
            u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, name);
            name[size - 1] = '\0';
            kart2DCommon->changeUnicodeTexture(name, 16, m_entryScreens[i], "Value", true);
        } else {
            kart2DCommon->changeNumberTexture<16>(m_values[i], m_entryScreens[i], "Value", true);
        }
    }

    for (u32 i = 0; i < SwitchCount; i++) {
        if (m_switchAnmTransformFrames[i] < 16) {
            if (i == 0 && m_switchAnmTransformFrames[i] == 8) {
                m_switchAnmTexPatternFrames[i] ^= 1;
            }
            m_switchAnmTransformFrames[i]++;
        }
    }
    for (u32 i = 0; i < m_chars.count(); i++) {
        u8 frame = i < m_charCount ? m_revealCode ? m_chars[i] : 9 : 10;
        m_charAnmTextureSRTKeyFrames[i] = frame;
        m_charAnmTexPatternFrames[i] = frame;
    }
    m_playerCircleAnmTransformFrame = (m_playerCircleAnmTransformFrame + 1) % 60;
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
    for (u32 i = 0; i < m_optionCount; i++) {
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
    for (u32 i = 0; i < m_switchAnmTransforms.count(); i++) {
        m_switchAnmTransforms[i]->m_frame = m_switchAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_switchAnmTexPatterns.count(); i++) {
        m_switchAnmTexPatterns[i]->m_frame = m_switchAnmTexPatternFrames[i];
    }
    for (u32 i = 0; i < m_charAnmTextureSRTKeys.count(); i++) {
        m_charAnmTextureSRTKeys[i]->m_frame = m_charAnmTextureSRTKeyFrames[i];
    }
    for (u32 i = 0; i < m_charAnmTexPatterns.count(); i++) {
        m_charAnmTexPatterns[i]->m_frame = m_charAnmTexPatternFrames[i];
    }
    for (u32 i = 0; i < m_playerAnmTransforms.count(); i++) {
        m_playerAnmTransforms[i]->m_frame = m_playerAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_playerNameAnmTransforms.count(); i++) {
        m_playerNameAnmTransforms[i]->m_frame = m_playerNameAnmTransformFrames[i];
    }
    m_playerCircleAnmTransform->m_frame = m_playerCircleAnmTransformFrame;
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
    for (u32 i = 0; i < m_charScreens.count(); i++) {
        m_charScreens[i].animationMaterials();
    }
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        m_playerScreens[i].animationMaterials();
    }
    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        m_entryScreens[i].animationMaterials();
    }

    if (m_isHost) {
        for (u32 i = 0; i < m_optionCount; i++) {
            RoomOption::Write(m_options[i], m_writeInfo.options, m_values[i]);
        }
        m_writeInfo.entryIndex = m_entryIndex;
    }
    client->writeStateRoom(m_writeInfo);
}

bool ScenePersonalRoom::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    return true;
}

bool ScenePersonalRoom::clientStatePack(const ClientStatePackReadInfo & /* readInfo */) {
    return true;
}

bool ScenePersonalRoom::clientStateRoom(const ClientStateRoomReadInfo &readInfo) {
    m_ok = m_ok && readInfo.ok;
    const Optional<ClientStateRoomReadInfo::Info> &info = readInfo.info;
    if (!info) {
        return true;
    }

    bool prevIsReady = m_isReady;
    m_isReady = true;

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    m_kartCount = info->kartCount;
    OnlineInfo &onlineInfo = OnlineInfo::Instance();
    onlineInfo.m_karts = info->karts;
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        const Kart &kart = onlineInfo.m_karts[i];
        J2DScreen &screen = m_playerScreens[i];
        screen.m_isVisible = i < info->kartCount;
        if (i >= info->kartCount) {
            continue;
        }
        for (u32 j = 0; j < kart.players.count(); j++) {
            const Player &player = kart.players[j];
            Array<char, 32> prefix;
            snprintf(prefix.values(), prefix.count(), "PName%" PRIu32, j);
            kart2DCommon->changeUnicodeTexture(player.name.values(), 3, screen, prefix.values());
        }
        if (kart.playerCount == 2) {
            m_playerNameAnmTransformFrames[i] = 0;
        } else {
            m_playerNameAnmTransformFrames[i] = 4;
        }
    }

    kart2DCommon->changeNumberTexture(info->spectatorCount, 3, m_mainScreen, "SCount");

    u32 raceMode = Modes[info->modeIndex];
    m_isRace = RaceMode::IsRace(raceMode);
    for (u32 i = 0; i < m_entryScreens.count(); i++) {
        J2DPicture *iconPicture = m_entryScreens[i].search("Icon")->downcast<J2DPicture>();
        const char *iconTextureName = ModeIconTextureNames[info->modeIndex];
        iconPicture->changeTexture(iconTextureName, 0);
    }

    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const CourseManager *courseManager = CourseManager::Instance();
    Optional<u32> packIndex =
            courseManager->searchPack(true, m_isRace, info->packCourseCount, info->packHash);
    if (packIndex) {
        sequenceInfo.m_packIndex = *packIndex;
        const CourseManager::Pack &pack = courseManager->pack(true, m_isRace, *packIndex);
        kart2DCommon->changeUnicodeTexture(pack.name(), 26, m_mainScreen, "Name");
    } else {
        m_ok = false;
    }

    m_charCount = 1;
    for (u32 i = 0; i < m_chars.count(); i++) {
        m_chars[i] = info->roomCode >> (i * 3) & 7;
        if (m_chars[i]) {
            m_charCount = i + 2;
        }
    }
    m_charCount = AlignUp(m_charCount, 5);
    if (m_charCount < m_chars.count()) {
        m_chars[m_charCount - 1] = 8;
    } else {
        m_charCount = m_chars.count();
    }

    if (!prevIsReady || m_switchAnmTransformFrames[1] == 8) {
        if (info->spectatingCounter == m_writeInfo.spectatingCounter) {
            m_writeInfo.spectating = info->spectating;
            m_switchAnmTexPatternFrames[1] = info->spectating;
        }
    }
    u32 localKartCount = 0, localPlayerCount = 0;
    for (u32 i = 0; i < m_kartCount; i++) {
        const Kart &kart = onlineInfo.m_karts[i];
        if (kart.local) {
            onlineInfo.m_localKartIndices[localKartCount++] = i;
            localPlayerCount += kart.playerCount;
        }
    }
    onlineInfo.m_localKartCount = localKartCount;
    onlineInfo.m_localPlayerCount = localPlayerCount;
    if (info->spectating) {
        m_ok = m_ok && localKartCount == 0 && info->spectatorCount >= sequenceInfo.m_padCount;
    } else {
        if (info->continuing && info->options.format == RoomOptionFormat::Duel) {
            m_ok = m_ok && localKartCount <= sequenceInfo.m_statusCount;
        } else {
            m_ok = m_ok && localKartCount == sequenceInfo.m_statusCount;
        }
        for (u32 i = 0, j = 0; i < m_kartCount && j < localKartCount; i++) {
            const Kart &kart = onlineInfo.m_karts[i];
            if (kart.local) {
                m_ok = m_ok && kart == onlineInfo.m_localKarts[j++];
            }
        }
    }
    onlineInfo.m_spectating = localKartCount == 0;

    if (!prevIsReady) {
        if (info->isRace == m_isRace) {
            m_options[m_optionCount++] = RoomOption::CodeType;
            m_options[m_optionCount++] = RoomOption::Format;
            if (m_isRace) {
                m_options[m_optionCount++] = RoomOption::EngineSize;
                m_options[m_optionCount++] = RoomOption::ItemMode;
                m_options[m_optionCount++] = RoomOption::LapCount;
                m_options[m_optionCount++] = RoomOption::RaceCount;
                m_options[m_optionCount++] = RoomOption::CourseSelection;
            } else {
                m_options[m_optionCount++] = RoomOption::BattleCount;
                m_options[m_optionCount++] = RoomOption::StageSelection;
            }
            for (u32 i = 0; i < m_optionCount; i++) {
                m_values[i] = RoomOption::DefaultValue(m_options[i]);
            }
        } else {
            m_ok = false;
        }
    }
    if (m_isHost) {
        m_canContinue = true;
        for (u32 i = 0; i < m_optionCount; i++) {
            u32 serverValue = RoomOption::Read(m_options[i], info->options);
            u32 clientValue = RoomOption::Read(m_options[i], m_writeInfo.options);
            m_canContinue = m_canContinue && serverValue == clientValue;
        }
    } else {
        for (u32 i = 0; i < m_optionCount; i++) {
            u8 value = RoomOption::Read(m_options[i], info->options);
            s32 direction = RoomOption::Direction(m_options[i], m_values[i], value);
            if (direction != 0) {
                if (direction < 0) {
                    m_entryLeftAnmTransformFrames[i] = 1;
                    m_entryLeftAnmTevRegKeyFrames[i] = 2;
                } else {
                    m_entryRightAnmTransformFrames[i] = 1;
                    m_entryRightAnmTevRegKeyFrames[i] = 2;
                }
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
            }
            m_values[i] = value;
        }
        if (info->entryIndex != m_entryIndex) {
            if (info->entryIndex < m_optionCount) {
                m_entryIndex = info->entryIndex;
            } else {
                m_entryIndex = MaxEntryCount - 1;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    }

    const RoomOptions &options = m_isHost ? m_writeInfo.options : info->options;
    u8 format = options.format;
    u8 maxTeamSize;
    switch (format) {
    case RoomOptionFormat::TeamsOf2:
        maxTeamSize = 2;
        break;
    case RoomOptionFormat::TeamsOf4:
        maxTeamSize = 4;
        break;
    default:
        maxTeamSize = 1;
        break;
    }
    bool isFFA = maxTeamSize == 1;
    u8 teamCount = (m_kartCount + maxTeamSize - 1) / maxTeamSize;
    if (!isFFA) {
        teamCount = Max<u8>(teamCount, 2);
    }
    maxTeamSize = (m_kartCount + teamCount - 1) / teamCount;
    onlineInfo.m_isFFA = isFFA;
    onlineInfo.m_teamCount = teamCount;
    onlineInfo.m_maxTeamSize = maxTeamSize;
    bool isDuel = info->options.format == RoomOptionFormat::Duel;
    onlineInfo.m_isDuel = isDuel;

    u8 courseSelection = options.courseSelection;
    switch (courseSelection) {
    case RoomOptionCourseSelection::Poll:
        onlineInfo.m_hasCourseSelection = !info->spectating;
        break;
    case RoomOptionCourseSelection::Host:
        onlineInfo.m_hasCourseSelection = onlineInfo.m_isHost;
        break;
    case RoomOptionCourseSelection::Random:
        onlineInfo.m_hasCourseSelection = false;
        break;
    }
    onlineInfo.m_hasCourseShuffle = courseSelection != RoomOptionCourseSelection::Poll;

    m_continuing = info->continuing;
    if (m_continuing) {
        if (isDuel) {
            m_ok = m_ok && info->kartCount == 2;
        } else {
            m_ok = m_ok && info->kartCount >= 2;
        }
    }

    if (!m_ok) {
        return true;
    }

    RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = isDuel ? 2 : info->spectating ? 1 : localKartCount;
    u32 consoleCount = statusCount == 3 ? 4 : statusCount;
    raceInfo.setRace(raceMode, m_kartCount, m_kartCount, consoleCount, statusCount);
    raceInfo.setRaceLevel(info->options.engineSize);
    raceInfo.m_lanLapNum = info->options.lapCount;
    for (u32 i = 0; i < consoleCount; i++) {
        const Kart &kart = onlineInfo.m_karts[i];
        bool spectating = isDuel ? !kart.local : info->spectating || i >= statusCount;
        u32 targetKart = isDuel ? i : spectating ? 0 : onlineInfo.m_localKartIndices[i];
        raceInfo.setConsoleTarget(i, targetKart, spectating);
    }
    return true;
}

void ScenePersonalRoom::clientStateError() {
    ErrorViewApp::Call(6);
}

void ScenePersonalRoom::wait() {
    m_ok = true;
    m_isReady = false;
    m_state = &ScenePersonalRoom::stateWait;
}

void ScenePersonalRoom::slideIn() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < m_options.count(); i++) {
        m_mainScreen.search("Option%u", i)->m_isVisible = i < m_optionCount;
        if (i < m_optionCount) {
            Array<char, 32> path;
            snprintf(path.values(), path.count(), "/optionnames/%u.txt", m_options[i]);
            char *name =
                    static_cast<char *>(ResMgr::GetPtr(ResMgr::ArchiveID::MRAMLoc, path.values()));
            u32 size = ResMgr::GetResSize(ResMgr::ArchiveID::MRAMLoc, name);
            name[size - 1] = '\0';
            kart2DCommon->changeUnicodeTexture(name, 20, m_entryScreens[i], "Name");
        }
    }

    MenuTitleLine::Instance()->drop("PersonalRoom.bti");
    m_state = &ScenePersonalRoom::stateSlideIn;
}

void ScenePersonalRoom::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &ScenePersonalRoom::stateSlideOut;
}

void ScenePersonalRoom::idle() {
    m_state = &ScenePersonalRoom::stateIdle;
}

void ScenePersonalRoom::nextScene() {
    m_state = &ScenePersonalRoom::stateNextScene;
}

void ScenePersonalRoom::stateWait() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_B || !m_ok) {
        if (m_isSearch) {
            m_nextScene = SceneType::FormatSelect;
        } else {
            m_nextScene = m_isHost ? SceneType::PackSelect : SceneType::RoomCodeEnter;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (m_isReady) {
        if (m_isSearch) {
            if (OnlineInfo::Instance().m_isFFA) {
                m_nextScene = SceneType::PlayerList;
            } else {
                m_nextScene = SceneType::TeamSelect;
            }
            nextScene();
        } else {
            slideIn();
        }
    }
}

void ScenePersonalRoom::stateSlideIn() {
    if (m_mainAnmTransformFrame < 26) {
        m_mainAnmTransformFrame++;
    } else {
        idle();
    }
}

void ScenePersonalRoom::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
    } else {
        nextScene();
    }
}

void ScenePersonalRoom::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if ((m_isHost && button.risingEdge() & PAD_BUTTON_A && m_entryIndex + 1 == MaxEntryCount) ||
            m_continuing) {
        if (m_isHost) {
            m_writeInfo.continuing = true;
        }
        if (OnlineInfo::Instance().m_isFFA) {
            m_nextScene = SceneType::PlayerList;
        } else {
            m_nextScene = SceneType::TeamSelect;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B || !m_ok) {
        m_nextScene = SceneType::RoomTypeSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.risingEdge() & (PAD_BUTTON_Y | PAD_BUTTON_X)) {
        if (m_switchAnmTransformFrames[1] == 16) {
            m_writeInfo.spectatingCounter++;
            m_writeInfo.spectating = !m_writeInfo.spectating;
            m_switchAnmTransformFrames[1] = 1;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_ATTENTION);
        }
    } else if (button.risingEdge() & PAD_TRIGGER_Z) {
        if (m_switchAnmTransformFrames[0] == 16) {
            m_revealCode = !m_revealCode;
            m_switchAnmTransformFrames[0] = 1;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_ATTENTION);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_entryIndex >= 1) {
            if (m_entryIndex + 1 == MaxEntryCount) {
                m_entryIndex = m_optionCount - 1;
            } else {
                m_entryIndex--;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_entryIndex + 1 < MaxEntryCount) {
            if (m_entryIndex + 1 == m_optionCount) {
                m_entryIndex = MaxEntryCount - 1;
            } else {
                m_entryIndex++;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        if (m_entryIndex < m_optionCount) {
            u8 option = m_options[m_entryIndex];
            u8 value = m_values[m_entryIndex];
            value = RoomOption::PrevValue(option, value);
            m_values[m_entryIndex] = value;
            m_entryLeftAnmTransformFrames[m_entryIndex] = 1;
            m_entryLeftAnmTevRegKeyFrames[m_entryIndex] = 2;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    } else if (m_isHost && button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        if (m_entryIndex < m_optionCount) {
            u8 option = m_options[m_entryIndex];
            u8 value = m_values[m_entryIndex];
            value = RoomOption::NextValue(option, value);
            m_values[m_entryIndex] = value;
            m_entryRightAnmTransformFrames[m_entryIndex] = 1;
            m_entryRightAnmTevRegKeyFrames[m_entryIndex] = 2;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
        }
    }
}

void ScenePersonalRoom::stateNextScene() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (m_nextScene == SceneType::TeamSelect || m_nextScene == SceneType::PlayerList) {
        if (button.risingEdge() & PAD_BUTTON_B || !m_ok) {
            if (m_isSearch) {
                m_nextScene = SceneType::FormatSelect;
            } else {
                m_nextScene = SceneType::RoomTypeSelect;
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        }
    }

    if (m_nextScene == SceneType::TeamSelect || m_nextScene == SceneType::PlayerList) {
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
