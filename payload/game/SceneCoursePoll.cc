#include "SceneCoursePoll.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

extern "C" {
#include <dolphin/OSTime.h>
}
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>
#include <payload/Lock.hh>

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
    for (u32 i = 0; i < MaxPlayerCount; i++) {
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
    uintptr_t msg = false;
    OSSendMessage(&m_queue, reinterpret_cast<void *>(msg), OS_MESSAGE_NOBLOCK);
    OSJoinThread(&m_loadThread, nullptr);
}

void SceneCoursePoll::init() {
    J2DPicture *iconPicture = m_mainScreen.search("BtlPict")->downcast<J2DPicture>();
    J2DPicture *namePicture = m_mainScreen.search("SubM")->downcast<J2DPicture>();
    RaceInfo &raceInfo = RaceInfo::Instance();
    const char *iconTextureName = RaceMode::IconTextureName(raceInfo.m_raceMode);
    iconPicture->changeTexture(iconTextureName, 0);
    const char *nameTextureName = RaceMode::NameTextureName(raceInfo.m_raceMode);
    namePicture->changeTexture(nameTextureName, 0);

    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    CourseManager *courseManager = CourseManager::Instance();
    if (raceInfo.isRace()) {
        m_courseCount = courseManager->raceCourseCount(sequenceInfo.m_packIndex);
    } else {
        m_courseCount = courseManager->battleCourseCount(sequenceInfo.m_packIndex);
    }
    m_playerCount = 8;
    m_playerIndex = UINT32_MAX;
    m_nameCount = 0;

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
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    if (m_nameCount < m_playerCount) {
        if (OSGetTime() % 30 == m_nameCount % 30) {
            J2DScreen &screen = m_courseScreens[m_nameCount];
            kart2DCommon->changeUnicodeTexture("ABC", 3, screen, "PName0");
            if (OSGetTime() / 30 % 2) {
                kart2DCommon->changeUnicodeTexture("DEF", 3, screen, "PName1");
                m_playerNameAnmTransformFrames[m_nameCount] = 0;
            } else {
                kart2DCommon->changeUnicodeTexture("   ", 3, screen, "PName1");
                m_playerNameAnmTransformFrames[m_nameCount] = 6;
            }
            J2DPicture::CornerColors cornerColors = Race2D::GetCornerColors(m_nameCount);
            for (u32 i = 0; i < 2; i++) {
                for (u32 j = 0; j < 3; j++) {
                    J2DPicture *picture = screen.search("PName%u%u", i, j)->downcast<J2DPicture>();
                    picture->m_cornerColors = cornerColors;
                }
            }
            if (m_nameCount == 1) {
                Lock<Mutex> lock(m_mutex);
                m_courseIndices[m_nameCount] = OSGetTime() % m_courseCount;
                refreshCourses();
            }
            m_nameCount++;
            if (m_nameCount == m_playerCount) {
                Lock<Mutex> lock(m_mutex);
                for (u32 i = 0; i < m_playerCount; i++) {
                    if (m_courseIndices[i] >= m_courseCount) {
                        m_courseIndices[i] = OSGetTime() % m_courseCount;
                    }
                }
                refreshCourses();
            }
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    }

    (this->*m_state)();

    for (u32 i = 0; i < 3; i += 2) {
        J2DPicture *picture = m_mainScreen.search("PCount%u", i)->downcast<J2DPicture>();
        s32 number = i == 0 ? m_nameCount : m_playerCount;
        kart2DCommon->changeNumberTexture(number, &picture, 1, false, false);
    }
    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        Lock<Mutex> lock(m_mutex);
        ResTIMG *thumbnail = nullptr, *nameImage = nullptr;
        for (u32 j = 0; j < m_courseIndices.count(); j++) {
            if (m_courseIndices[j] != m_courseIndices[i]) {
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

    for (u32 i = 0; i < m_playerCount; i++) {
        if (i == m_playerIndex) {
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
        m_mainScreen.search("PCount%u", i)->setAlpha(m_playerCountAlpha);
    }

    m_mainScreen.animation();
    m_gridScreen.animation();
    for (u32 i = 0; i < m_courseScreens.count(); i++) {
        m_courseScreens[i].animationMaterials();
    }
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
    m_playerCountAlpha = 0;
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
        m_thumbnailAnmTevRegKeyFrames[i] = i == m_playerIndex;
    }
    m_state = &SceneCoursePoll::stateSelect;
}

void SceneCoursePoll::nextScene() {
    for (u32 i = 0; i < m_thumbnails.count(); i++) {
        m_thumbnails[i].reset();
    }
    for (u32 i = 0; i < m_nameImages.count(); i++) {
        m_nameImages[i].reset();
    }
    refreshCourses();
    m_state = &SceneCoursePoll::stateNextScene;
}

void SceneCoursePoll::stateSlideIn() {
    if (m_mainAnmTransformFrame < 10) {
        m_mainAnmTransformFrame++;
        m_gridAnmTransformFrame = m_mainAnmTransformFrame;
        m_playerCountAlpha = (Max<u32>(m_mainAnmTransformFrame, 5) - 5) * 51;
    } else {
        idle();
    }
}

void SceneCoursePoll::stateSlideOut() {
    uintptr_t msg = false;
    OSSendMessage(&m_queue, reinterpret_cast<void *>(msg), OS_MESSAGE_NOBLOCK);
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_gridAnmTransformFrame = m_mainAnmTransformFrame;
        m_playerCountAlpha = (Max<u32>(m_mainAnmTransformFrame, 5) - 5) * 51;
    } else {
        if (OSIsThreadTerminated(&m_loadThread)) {
            OSDetachThread(&m_loadThread);
            m_loadStack.reset();
            nextScene();
        }
    }
}

void SceneCoursePoll::stateIdle() {
    for (u32 i = 0; i < m_playerCount; i++) {
        if (m_courseIndices[i] >= m_courseCount) {
            return;
        }
    }
    spin();
}

void SceneCoursePoll::stateSpin() {
    m_spinFrame++;
    if (m_spinFrame < 180) {
        if (m_spinFrame % 5 == 0) {
            m_playerIndex = OSGetTime() % m_playerCount;
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        select();
    }
}

void SceneCoursePoll::stateSelect() {
    if (m_selectFrame < 120) {
        m_selectFrame++;
    } else {
        GameAudio::Main::Instance()->fadeOutAll(15);
        System::GetDisplay()->startFadeOut(15);
        slideOut();
    }
}

void SceneCoursePoll::stateNextScene() {}

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

    uintptr_t msg = true;
    OSSendMessage(&m_queue, reinterpret_cast<void *>(msg), OS_MESSAGE_NOBLOCK);
}

void *SceneCoursePoll::load() {
    while (true) {
        uintptr_t msg;
        OSReceiveMessage(&m_queue, reinterpret_cast<void **>(&msg), OS_MESSAGE_BLOCK);
        if (!msg) {
            return nullptr;
        }

        while (true) {
            Array<u32, MaxPlayerCount> courseIndices;
            {
                Lock<Mutex> lock(m_mutex);
                courseIndices = m_courseIndices;
            }

            if (load(courseIndices)) {
                break;
            }

            if (OSReceiveMessage(&m_queue, reinterpret_cast<void **>(&msg), OS_MESSAGE_NOBLOCK)) {
                if (!msg) {
                    return nullptr;
                }
            }
        }
    }
}

bool SceneCoursePoll::load(const Array<u32, MaxPlayerCount> &courseIndices) {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    CourseManager *courseManager = CourseManager::Instance();
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
        const CourseManager::Course *course;
        if (RaceInfo::Instance().isRace()) {
            course = &courseManager->raceCourse(sequenceInfo.m_packIndex, courseIndex);
        } else {
            course = &courseManager->battleCourse(sequenceInfo.m_packIndex, courseIndex);
        }
        if (!thumbnail->get()) {
            void *texture = course->loadThumbnail(m_heap);
            {
                Lock<Mutex> lock(m_mutex);
                thumbnail->reset(static_cast<ResTIMG *>(texture));
            }
            return false;
        }
        if (!nameImage->get()) {
            void *texture = course->loadNameImage(m_heap);
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
