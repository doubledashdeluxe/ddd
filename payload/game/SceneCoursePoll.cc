#include "SceneCoursePoll.hh"

#include "game/AppMgr.hh"
#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/MenuTitleLine.hh"
#include "game/Modes.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/Race2D.hh"
#include "game/RaceApp.hh"
#include "game/RaceInfo.hh"
#include "game/ResMgr.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>
#include <payload/Lock.hh>
#include <payload/crypto/CubeRandom.hh>
#include <payload/online/CubeClient.hh>

SceneCoursePoll::SceneCoursePoll(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap), m_heap(heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);

    m_mainScreen.set("CoursePoll.blo", 0x1040000, lanEntryArchive);
    m_gridScreen.set("CoursePollGrid.blo", 0x1040000, lanEntryArchive);
    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        m_courseScreens[i].set("CoursePollCourse.blo", 0x1040000, lanEntryArchive);
    }

    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        m_gridScreen.search("Course%u", i)->appendChild(&m_courseScreens[i]);
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", m_archive);
    m_mainScreen.search("NSlMap")->setAnimation(m_mainAnmTransform);
    m_gridAnmTransform = J2DAnmLoaderDataBase::Load("CoursePollGrid.bck", lanEntryArchive);
    m_gridScreen.setAnimation(m_gridAnmTransform);
    for (u32 i = 0; i < m_courseAnmTransforms.count(); i++) {
        m_courseAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("CoursePollCourse.bck", lanEntryArchive);
        m_courseScreens[i].search("NMap")->setAnimation(m_courseAnmTransforms[i]);
        m_courseScreens[i].search("Name")->setAnimation(m_courseAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_playerNameAnmTransforms.count(); i++) {
        m_playerNameAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("CoursePollCourse.bck", lanEntryArchive);
        m_courseScreens[i].search("PName")->setAnimation(m_playerNameAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_highlightAnmTransforms.count(); i++) {
        m_highlightAnmTransforms[i] =
                J2DAnmLoaderDataBase::Load("CoursePollCourse.bck", lanEntryArchive);
        m_courseScreens[i].search("HighL")->setAnimation(m_highlightAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_highlightAnmColors.count(); i++) {
        m_highlightAnmColors[i] = J2DAnmLoaderDataBase::Load("SelectMap1.bpk", m_archive);
        m_highlightAnmColors[i]->searchUpdateMaterialID(&m_courseScreens[i]);
        m_courseScreens[i].search("HighL")->setAnimation(m_highlightAnmColors[i]);
    }
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeys.count(); i++) {
        m_thumbnailAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SelectMap1.brk", m_archive);
        m_thumbnailAnmTevRegKeys[i]->searchUpdateMaterialID(&m_courseScreens[i]);
        m_courseScreens[i].search("MapPict")->setAnimation(m_thumbnailAnmTevRegKeys[i]);
    }
    m_courseNameAnmTransform = J2DAnmLoaderDataBase::Load("CoursePollCourse.bck", lanEntryArchive);
    for (u32 i = 0; i < MaxRoomKartCount; i++) {
        m_courseScreens[i].search("NName")->setAnimation(m_courseNameAnmTransform);
    }
    for (u32 i = 0; i < m_courseNameAnmTevRegKeys.count(); i++) {
        m_courseNameAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SelectMap1.brk", m_archive);
        m_courseNameAnmTevRegKeys[i]->searchUpdateMaterialID(&m_courseScreens[i]);
        m_courseScreens[i].search("Name")->setAnimation(m_courseNameAnmTevRegKeys[i]);
    }

    m_courseAnmTransformFrames.fill(0);
    m_playerNameAnmTransformFrames.fill(0);
    m_highlightAnmTransformFrames.fill(0);
    m_highlightAnmColorFrames.fill(0);
    m_courseNameAnmTevRegKeyFrames.fill(0);
}

SceneCoursePoll::~SceneCoursePoll() {
    m_loading = false;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
    OSJoinThread(&m_loadThread, nullptr);
}

void SceneCoursePoll::init() {
    J2DPicture *iconPicture = m_mainScreen.search("BtlPict")->downcast<J2DPicture>();
    J2DPicture *namePicture = m_mainScreen.search("SubM")->downcast<J2DPicture>();
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const char *iconTextureName = ModeIconTextureNames[onlineInfo.m_modeIndex];
    iconPicture->changeTexture(iconTextureName, 0);
    const char *nameTextureName = ModeNameTextureNames[onlineInfo.m_modeIndex];
    namePicture->changeTexture(nameTextureName, 0);

    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    const CourseManager *courseManager = CourseManager::Instance();
    m_ok = true;
    m_courseCount = courseManager->courseCount(raceInfo.isRace(), sequenceInfo.m_packIndex);
    m_kartCount = raceInfo.getKartCount();
    m_kartIndex = UINT32_MAX;
    m_nameCount = 0;
    m_selectedKartIndex.reset();
    for (u32 i = 0; i < m_courseShuffleIndices.count(); i++) {
        m_courseShuffleIndices[i] = i;
    }

    m_writeInfo.packCourseCount = m_courseCount;
    m_writeInfo.kartCount = m_kartCount;
    ClientStatePollWriteInfo::Ready &ready = m_writeInfo.ready.emplace();
    ready.kartCount = onlineInfo.m_spectating ? 0 : raceInfo.getStatusCount();
    for (s16 i = 0; i < ready.kartCount; i++) {
        ready.karts[i].characterIDs = onlineInfo.m_characterIDs[i];
        ready.karts[i].kartID = onlineInfo.m_kartIDs[i];
    }
    if (onlineInfo.m_hasCourseSelection) {
        ready.courseIndex = sequenceInfo.m_mapIndex;
    }

    m_courseAlphas.fill(0);

    slideIn();
}

void SceneCoursePoll::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_gridScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneCoursePoll::calc() {
    CubeClient *client = CubeClient::Instance();
    client->read(*this);

    (this->*m_state)();

    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < 3; i += 2) {
        J2DPicture *picture = m_mainScreen.search("PCount%u", i)->downcast<J2DPicture>();
        s32 number = i == 0 ? m_nameCount : m_kartCount;
        kart2DCommon->changeNumberTexture(number, &picture, 1, false, false);
    }
    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        u32 shuffleIndex = m_courseShuffleIndices[i];
        Lock<Mutex> lock(m_mutex);
        ResTIMG *thumbnail = nullptr, *nameImage = nullptr;
        for (u32 j = 0; j < m_courseIndices.count(); j++) {
            if (m_courseIndices[j] != m_courseIndices[shuffleIndex]) {
                continue;
            }
            if (!thumbnail) {
                thumbnail = m_thumbnails[j].get();
            }
            if (!nameImage) {
                nameImage = m_nameImages[j].get();
            }
        }
        if (thumbnail) {
            J2DPicture *picture = m_courseScreens[i].search("MapPict")->downcast<J2DPicture>();
            picture->m_isVisible = true;
            picture->changeTexture(thumbnail, 0);
        }
        if (nameImage) {
            J2DPicture *picture = m_courseScreens[i].search("Name")->downcast<J2DPicture>();
            picture->m_isVisible = true;
            picture->changeTexture(nameImage, 0);
        }
    }

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    for (u32 i = 0; i < m_kartCount; i++) {
        if (i == m_kartIndex) {
            if (m_courseAnmTransformFrames[i] < 8) {
                m_courseAnmTransformFrames[i]++;
            }
            m_highlightAnmTransformFrames[i] = (m_highlightAnmTransformFrames[i] + 1) % 60;
            m_highlightAnmColorFrames[i] = (m_highlightAnmColorFrames[i] + 1) % 60;
            m_courseNameAnmTevRegKeyFrames[i] = 1 + m_courseNameAnmTevRegKeyFrames[i] % 59;
        } else {
            if (m_courseAnmTransformFrames[i] > 0) {
                m_courseAnmTransformFrames[i]--;
            }
            m_highlightAnmTransformFrames[i] = 0;
            m_highlightAnmColorFrames[i] = 0;
            m_courseNameAnmTevRegKeyFrames[i] = 0;
        }
    }
    for (u32 i = 0; i < m_nameCount; i++) {
        if (m_courseAlphas[i] < 255) {
            m_courseAlphas[i] += 51;
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_gridAnmTransform->m_frame = m_gridAnmTransformFrame;
    for (u32 i = 0; i < m_courseAnmTransforms.count(); i++) {
        m_courseAnmTransforms[i]->m_frame = m_courseAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_playerNameAnmTransforms.count(); i++) {
        m_playerNameAnmTransforms[i]->m_frame = m_playerNameAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_highlightAnmTransforms.count(); i++) {
        m_highlightAnmTransforms[i]->m_frame = m_highlightAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_highlightAnmColors.count(); i++) {
        m_highlightAnmColors[i]->m_frame = m_highlightAnmColorFrames[i];
    }
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeys.count(); i++) {
        m_thumbnailAnmTevRegKeys[i]->m_frame = m_thumbnailAnmTevRegKeyFrames[i];
    }
    m_courseNameAnmTransform->m_frame = m_courseNameAnmTransformFrame;
    for (u32 i = 0; i < m_courseNameAnmTevRegKeys.count(); i++) {
        m_courseNameAnmTevRegKeys[i]->m_frame = m_courseNameAnmTevRegKeyFrames[i];
    }

    for (u32 i = 0; i < m_courseAlphas.count(); i++) {
        for (u32 j = 0; j < 2; j++) {
            for (u32 k = 0; k < 3; k++) {
                m_courseScreens[i].search("PName%u%u", j, k)->setAlpha(m_courseAlphas[i]);
            }
        }
        m_courseScreens[i].search("MapPict")->setAlpha(m_courseAlphas[i]);
        m_courseScreens[i].search("WMap")->setAlpha(m_courseAlphas[i]);
        m_courseScreens[i].search("Name")->setAlpha(m_courseAlphas[i]);
        if (m_courseAlphas[i] != 255) {
            m_highlightAnmColors[i]->m_frame = (255 - m_courseAlphas[i]) * 59 / 255;
        }
    }
    for (u32 i = 0; i < 3; i++) {
        m_mainScreen.search("PCount%u", i)->setAlpha(m_kartCountAlpha);
    }

    m_mainScreen.animation();
    m_gridScreen.animation();
    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        m_courseScreens[i].animationMaterials();
    }

    client->writeStatePoll(m_writeInfo);
}

bool SceneCoursePoll::clientStatePoll(const ClientStatePollReadInfo &readInfo) {
    m_ok = m_ok && readInfo.ok;

    const Ring<u8, MaxRoomKartCount> &kartIndices = readInfo.kartIndices;
    for (u32 i = 0; i < kartIndices.count(); i++) {
        for (u32 j = i + 1; j < kartIndices.count(); j++) {
            if (kartIndices[j] == kartIndices[i]) {
                m_ok = false;
            }
        }
    }
    if (!m_ok) {
        return true;
    }

    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (; m_nameCount < kartIndices.count(); m_nameCount++) {
        u32 kartIndex = kartIndices[m_nameCount];
        J2DScreen &screen = m_courseScreens[m_nameCount];
        const Kart &kart = onlineInfo.m_karts[kartIndex];
        for (u32 i = 0; i < 2; i++) {
            Array<char, 32> prefix;
            snprintf(prefix.values(), prefix.count(), "PName%" PRIu32, i);
            const Player &player = kart.players[i];
            kart2DCommon->changeUnicodeTexture(player.name.values(), 3, screen, prefix.values());
        }
        m_playerNameAnmTransformFrames[m_nameCount] = kart.playerCount == 2 ? 0 : 6;
        u32 colorIndex = kartIndex;
        if (!onlineInfo.m_isFFA) {
            colorIndex = onlineInfo.m_teams[kartIndex];
        }
        J2DPicture::CornerColors cornerColors = Race2D::GetCornerColors(colorIndex);
        for (u32 i = 0; i < 2; i++) {
            for (u32 j = 0; j < 3; j++) {
                J2DPicture *picture = screen.search("PName%u%u", i, j)->downcast<J2DPicture>();
                picture->m_cornerColors = cornerColors;
            }
        }
        if (kart.local && onlineInfo.m_hasCourseSelection) {
            Lock<Mutex> lock(m_mutex);
            m_courseIndices[m_nameCount] = sequenceInfo.m_mapIndex;
            refreshCourses();
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }

    const Optional<ClientStatePollReadInfo::Ready> &ready = readInfo.ready;
    if (!ready) {
        return true;
    }

    RaceInfo &raceInfo = RaceInfo::Instance();
    for (u32 i = 0; i < m_kartCount; i++) {
        u32 kartIndex = kartIndices[i];
        const ClientStatePollReadInfo::Kart &kart = ready->karts[i];
        u8 frontCharacterID = kart.characterIDs[0];
        u8 backCharacterID = kart.characterIDs[1];
        if (frontCharacterID == backCharacterID) {
            m_ok = false;
        }
        u8 kartID = kart.kartID;
        if (kartID != KartID::Extra) {
            u32 frontCharacterWeight = KartInfo::GetDriverWeight(frontCharacterID + 1);
            u32 backCharacterWeight = KartInfo::GetDriverWeight(backCharacterID + 1);
            u32 maxCharacterWeight = Max(frontCharacterWeight, backCharacterWeight);
            u32 kartWeight = KartInfo::GetKartWeight(kartID);
            if (kartWeight != maxCharacterWeight) {
                m_ok = false;
            }
        }
        KartGamePad *frontPad = nullptr, *backPad = nullptr;
        u32 localKartCount = onlineInfo.m_spectating ? 0 : raceInfo.getStatusCount();
        for (u32 j = 0; j < localKartCount; j++) {
            if (onlineInfo.m_localKartIndices[j] != kartIndex) {
                continue;
            }
            const Array<u8, 2> &padIndices = onlineInfo.m_padIndices[j];
            frontPad = KartGamePad::GamePad(padIndices[0]);
            if (padIndices[1] != padIndices[0]) {
                backPad = KartGamePad::GamePad(padIndices[1]);
            }
        }
        raceInfo.setKart(kartIndex, kartID, frontCharacterID + 1, frontPad, backCharacterID + 1,
                backPad);
    }
    if (!m_ok) {
        return true;
    }

    m_selectedKartIndex = ready->kartIndex;
    CubeRandom *random = CubeRandom::Instance();
    Lock<Mutex> lock(m_mutex);
    for (u32 i = 0; i < ready->kartCount; i++) {
        const ClientStatePollReadInfo::Kart &kart = ready->karts[i];
        m_courseIndices[i] = kart.courseIndex;
    }
    for (u32 i = 0; i < m_courseIndices.count(); i++) {
        if (m_courseIndices[i] >= m_courseCount) {
            m_courseIndices[i] = random->get(m_courseCount);
        }
    }
    refreshCourses();
    return true;
}

void SceneCoursePoll::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneCoursePoll::slideIn() {
    RaceInfo &raceInfo = RaceInfo::Instance();
    if (raceInfo.isRace()) {
        MenuTitleLine::Instance()->drop("CoursePoll.bti");
    } else {
        MenuTitleLine::Instance()->drop("MapPoll.bti");
    }
    m_mainAnmTransformFrame = 0;
    m_gridAnmTransformFrame = 0;
    m_thumbnailAnmTevRegKeyFrames.fill(1);
    m_courseNameAnmTransformFrame = raceInfo.isRace() ? 0 : 6;
    m_kartCountAlpha = 0;
    m_loading = true;
    m_courseIndices.fill(UINT32_MAX);
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    u32 stackSize = 64 * 1024;
    m_loadStack.reset(new (m_heap, 0x8) u8[stackSize]);
    OSCreateThread(&m_loadThread, Load, this, m_loadStack.get() + stackSize, stackSize, 25, 0);
    OSResumeThread(&m_loadThread);
    refreshCourses();
    m_state = &SceneCoursePoll::stateSlideIn;
}

void SceneCoursePoll::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_loading = false;
    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
    m_state = &SceneCoursePoll::stateSlideOut;
}

void SceneCoursePoll::idle() {
    m_state = &SceneCoursePoll::stateIdle;
}

void SceneCoursePoll::spin() {
    m_spinFrame = 1;
    m_state = &SceneCoursePoll::stateSpin;
}

void SceneCoursePoll::select() {
    m_selectFrame = 1;
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeyFrames.count(); i++) {
        m_thumbnailAnmTevRegKeyFrames[i] = i == m_kartIndex;
    }
    m_state = &SceneCoursePoll::stateSelect;
}

void SceneCoursePoll::nextScene() {
    m_state = &SceneCoursePoll::stateNextScene;
}

void SceneCoursePoll::nextRace() {
    m_state = &SceneCoursePoll::stateNextRace;
}

void SceneCoursePoll::stateSlideIn() {
    if (m_mainAnmTransformFrame < 10) {
        m_mainAnmTransformFrame++;
        m_gridAnmTransformFrame = m_mainAnmTransformFrame;
        m_kartCountAlpha = (Max<u32>(m_mainAnmTransformFrame, 5) - 5) * 51;
    } else {
        idle();
    }
}

void SceneCoursePoll::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_gridAnmTransformFrame = m_mainAnmTransformFrame;
        m_kartCountAlpha = (Max<u32>(m_mainAnmTransformFrame, 5) - 5) * 51;
    } else {
        if (OSIsThreadTerminated(&m_loadThread)) {
            OSDetachThread(&m_loadThread);
            m_loadStack.reset();
            for (u32 i = 0; i < m_thumbnails.count(); i++) {
                m_thumbnails[i].reset();
            }
            for (u32 i = 0; i < m_nameImages.count(); i++) {
                m_nameImages[i].reset();
            }
            refreshCourses();
            if (m_nextScene == SceneType::None) {
                nextRace();
            } else {
                nextScene();
            }
        }
    }
}

void SceneCoursePoll::stateIdle() {
    if (!m_ok) {
        m_nextScene = SceneType::Title;
        GameAudio::Main::Instance()->fadeOutAll(15);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL);
        System::GetDisplay()->startFadeOut(15);
        slideOut();
        return;
    }

    if (m_selectedKartIndex) {
        SequenceInfo &sequenceInfo = SequenceInfo::Instance();
        const RaceInfo &raceInfo = RaceInfo::Instance();
        const CourseManager *courseManager = CourseManager::Instance();
        u32 courseIndex = m_courseIndices[*m_selectedKartIndex];
        const CourseManager::Course &course =
                courseManager->course(raceInfo.isRace(), sequenceInfo.m_packIndex, courseIndex);
        ResMgr::LoadExtendedCourseData(&course, 2);
        spin();
    }
}

void SceneCoursePoll::stateSpin() {
    m_spinFrame++;
    if (m_spinFrame >= 180) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        select();
    } else if (m_spinFrame % 5 == 0) {
        if (m_spinFrame == 180 - 5) {
            m_kartIndex = *m_selectedKartIndex;
            for (u32 i = 0; i < m_courseShuffleIndices.count(); i++) {
                m_courseShuffleIndices[i] = i;
            }
        } else {
            CubeRandom *random = CubeRandom::Instance();
            m_kartIndex = random->get(m_kartCount);
            if (OnlineInfo::Instance().m_hasCourseShuffle) {
                random->shuffle(m_courseShuffleIndices, m_courseShuffleIndices.count());
            }
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    }
}

void SceneCoursePoll::stateSelect() {
    if (m_selectFrame < 120) {
        m_selectFrame++;
    } else {
        m_nextScene = SceneType::None;
        GameAudio::Main::Instance()->fadeOutAll(15);
        System::GetDisplay()->startFadeOut(15);
        slideOut();
    }
}

void SceneCoursePoll::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneCoursePoll::stateNextRace() {
    if (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveID::Course)) {
        return;
    }

    AppMgr::Request(AppMgr::Request::DestroyApp);
    RaceApp::Call();
}

void SceneCoursePoll::refreshCourses() {
    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        Array<const char *, 2> names;
        names[0] = "MapPict";
        names[1] = "Name";
        for (u32 j = 0; j < names.count(); j++) {
            J2DPicture *picture = m_courseScreens[i].search(names[j])->downcast<J2DPicture>();
            picture->m_isVisible = false;
            picture->changeTexture("SelCourse_Pict_Box1.bti", 0);
        }
    }

    OSSendMessage(&m_queue, nullptr, OS_MESSAGE_NOBLOCK);
}

void *SceneCoursePoll::load() {
    while (m_loading) {
        OSReceiveMessage(&m_queue, nullptr, OS_MESSAGE_BLOCK);

        while (m_loading) {
            Array<u32, MaxRoomKartCount> courseIndices;
            {
                Lock<Mutex> lock(m_mutex);
                courseIndices = m_courseIndices;
            }

            if (load(courseIndices)) {
                break;
            }
        }
    }
    return nullptr;
}

bool SceneCoursePoll::load(const Array<u32, MaxRoomKartCount> &courseIndices) {
    const CourseManager *courseManager = CourseManager::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    for (u32 i = 0; i < courseIndices.count(); i++) {
        u32 courseIndex = courseIndices[i];
        if (courseIndex >= m_courseCount) {
            continue;
        }
        UniquePtr<ResTIMG> *thumbnail = nullptr, *nameImage = nullptr;
        for (u32 j = 0; j < courseIndices.count(); j++) {
            if (courseIndices[j] != courseIndices[i]) {
                continue;
            }
            if (!thumbnail || !thumbnail->get()) {
                thumbnail = &m_thumbnails[j];
            }
            if (!nameImage || !nameImage->get()) {
                nameImage = &m_nameImages[j];
            }
        }
        const CourseManager::Course &course =
                courseManager->course(raceInfo.isRace(), sequenceInfo.m_packIndex, courseIndex);
        if (!thumbnail->get()) {
            void *texture = course.loadThumbnail(m_heap);
            {
                Lock<Mutex> lock(m_mutex);
                thumbnail->reset(static_cast<ResTIMG *>(texture));
            }
            return false;
        }
        if (!nameImage->get()) {
            void *texture = course.loadNameImage(m_heap);
            {
                Lock<Mutex> lock(m_mutex);
                nameImage->reset(static_cast<ResTIMG *>(texture));
            }
            return false;
        }
    }
    return true;
}

void *SceneCoursePoll::Load(void *param) {
    return static_cast<SceneCoursePoll *>(param)->load();
}
