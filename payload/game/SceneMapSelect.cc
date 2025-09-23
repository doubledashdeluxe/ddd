#include "SceneMapSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineTimer.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"
#include "game/SystemRecord.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>
#include <payload/CourseManager.hh>
#include <payload/Lock.hh>
#include <payload/crypto/CubeRandom.hh>
#include <portable/Algorithm.hh>

SceneMapSelect::SceneMapSelect(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap), m_heap(heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *backgroundArchive = sceneFactory->archive(SceneFactory::ArchiveType::Background);
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);
    JKRArchive *ghostDataArchive = sceneFactory->archive(SceneFactory::ArchiveType::GhostData);

    if (!SequenceInfo::Instance().m_isOnline) {
        MenuBackground::Create(backgroundArchive);
    }
    MenuTitleLine::Create(titleLineArchive, heap);

    m_mainScreen.set("SelectMapLayout.blo", 0x1040000, m_archive);
    m_gridScreen.set("SelectMapGrid.blo", 0x1040000, m_archive);
    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        m_mapScreens[i].set("SelectMap1.blo", 0x1040000, m_archive);
    }
    m_arrowScreen.set("GDIndexLayout.blo", 0x20000, ghostDataArchive);

    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        m_gridScreen.search("Map%u", i)->appendChild(&m_mapScreens[i]);
    }
    m_arrowScreen.search("NSaveGD")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", m_archive);
    m_mainScreen.search("NSlMap")->setAnimation(m_mainAnmTransform);
    m_selectAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", m_archive);
    m_mainScreen.search("NRandom")->setAnimation(m_selectAnmTransform);
    m_mainScreen.search("OK_wb11")->setAnimation(m_selectAnmTransform);
    m_okAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("SelectMapLayout.btk", m_archive);
    m_okAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_okAnmTextureSRTKey);
    m_okAnmColor = J2DAnmLoaderDataBase::Load("SelectMapLayout.bpk", m_archive);
    m_okAnmColor->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_okAnmColor);
    m_gridAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapGrid.bck", m_archive);
    m_gridScreen.setAnimation(m_gridAnmTransform);
    for (u32 i = 0; i < m_mapAnmTransforms.count(); i++) {
        m_mapAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMap1.bck", m_archive);
        m_mapScreens[i].search("NMap")->setAnimation(m_mapAnmTransforms[i]);
        m_mapScreens[i].search("Name")->setAnimation(m_mapAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_highlightAnmTransforms.count(); i++) {
        m_highlightAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectMap1.bck", m_archive);
        m_mapScreens[i].search("HighL")->setAnimation(m_highlightAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_highlightAnmColors.count(); i++) {
        m_highlightAnmColors[i] = J2DAnmLoaderDataBase::Load("SelectMap1.bpk", m_archive);
        m_highlightAnmColors[i]->searchUpdateMaterialID(&m_mapScreens[i]);
        m_mapScreens[i].search("HighL")->setAnimation(m_highlightAnmColors[i]);
    }
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeys.count(); i++) {
        m_thumbnailAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SelectMap1.brk", m_archive);
        m_thumbnailAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mapScreens[i]);
        m_mapScreens[i].search("MapPict")->setAnimation(m_thumbnailAnmTevRegKeys[i]);
    }
    m_nameAnmTransform = J2DAnmLoaderDataBase::Load("SelectMap1.bck", m_archive);
    for (u32 i = 0; i < 9; i++) {
        m_mapScreens[i].search("NName")->setAnimation(m_nameAnmTransform);
    }
    for (u32 i = 0; i < m_nameAnmTevRegKeys.count(); i++) {
        m_nameAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SelectMap1.brk", m_archive);
        m_nameAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mapScreens[i]);
        m_mapScreens[i].search("Name")->setAnimation(m_nameAnmTevRegKeys[i]);
    }
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapArrow.bck", m_archive);
    for (u32 i = 0; i < 2; i++) {
        m_arrowScreen.search("MArrow%02u", i + 1)->setAnimation(m_arrowAnmTransform);
    }

    m_okAnmTextureSRTKeyFrame = 0;
    m_okAnmColorFrame = 0;
    m_mapAnmTransformFrames.fill(0);
    m_highlightAnmTransformFrames.fill(0);
    m_highlightAnmColorFrames.fill(0);
    m_nameAnmTevRegKeyFrames.fill(0);
    m_arrowAnmTransformFrame = 0;
}

SceneMapSelect::~SceneMapSelect() {
    uintptr_t msg = false;
    OSSendMessage(&m_queue, reinterpret_cast<void *>(msg), OS_MESSAGE_NOBLOCK);
    OSJoinThread(&m_loadThread, nullptr);
}

void SceneMapSelect::init() {
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
        m_mapCount = courseManager->raceCourseCount(sequenceInfo.m_packIndex);
    } else {
        m_mapCount = courseManager->battleCourseCount(sequenceInfo.m_packIndex);
    }
    m_mapIndex = 0;
    if (sequenceInfo.m_fromPause) {
        m_mapIndex = sequenceInfo.m_mapIndex;
        sequenceInfo.m_fromPause = false;
        System::GetDisplay()->startFadeIn(15);
        GameAudio::Main::Instance()->startSequenceBgm(SoundID::JA_BGM_SELECT);
        SequenceApp::Instance()->ready(SceneType::PackSelect);
    }
    m_rowCount = (m_mapCount + 3 - 1) / 3;
    m_rowIndex = m_mapIndex / 3;
    m_rowIndex = Min(m_rowIndex, m_rowCount - Min<u32>(m_rowCount, 2));

    slideIn();
}

void SceneMapSelect::draw() {
    m_graphContext->setViewport();

    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    if (sequenceInfo.m_isOnline) {
        OnlineBackground::Instance()->draw(m_graphContext);
    } else {
        MenuBackground::Instance()->draw(m_graphContext);
    }
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_gridScreen.draw(0.0f, 0.0f, m_graphContext);
    m_arrowScreen.draw(0.0f, 0.0f, m_graphContext);

    if (sequenceInfo.m_isOnline) {
        OnlineTimer::Instance()->draw(m_graphContext);
    }
}

void SceneMapSelect::calc() {
    (this->*m_state)();

    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        Lock<Mutex> lock(m_mutex);
        u32 mapIndex = m_rowIndex * 3 + i;
        if (mapIndex >= m_mapCount) {
            break;
        }
        u32 j;
        for (j = 0; j < m_nextMapIndices.count(); j++) {
            if (m_nextMapIndices[j] == mapIndex) {
                break;
            }
        }
        if (j == m_nextMapIndices.count()) {
            continue;
        }
        ResTIMG *thumbnail = nullptr, *nameImage = nullptr;
        for (u32 j = 0; j < m_currMapIndices.count(); j++) {
            if (m_currMapIndices[j] == mapIndex) {
                thumbnail = m_thumbnails[j].get();
                nameImage = m_nameImages[j].get();
                break;
            }
        }
        if (thumbnail) {
            J2DPicture *picture = m_mapScreens[i].search("MapPict")->downcast<J2DPicture>();
            picture->m_isVisible = true;
            picture->changeTexture(thumbnail, 0);
        }
        if (nameImage) {
            J2DPicture *picture = m_mapScreens[i].search("Name")->downcast<J2DPicture>();
            picture->m_isVisible = true;
            picture->changeTexture(nameImage, 0);
        }
    }

    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    if (sequenceInfo.m_isOnline) {
        OnlineBackground::Instance()->calc();
    } else {
        MenuBackground::Instance()->calc();
    }
    MenuTitleLine::Instance()->calc();

    m_okAnmTextureSRTKeyFrame = (m_okAnmTextureSRTKeyFrame + 1) % 120;
    m_okAnmColorFrame = (m_okAnmColorFrame + 1) % 120;
    for (u32 i = 0; i < 9; i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        if (mapIndex == m_mapIndex) {
            if (m_mapAnmTransformFrames[i] < 8) {
                m_mapAnmTransformFrames[i]++;
            }
            m_highlightAnmTransformFrames[i] = (m_highlightAnmTransformFrames[i] + 1) % 60;
            m_highlightAnmColorFrames[i] = (m_highlightAnmColorFrames[i] + 1) % 60;
            m_nameAnmTevRegKeyFrames[i] = 1 + m_nameAnmTevRegKeyFrames[i] % 59;
        } else {
            if (m_mapAnmTransformFrames[i] > 0) {
                m_mapAnmTransformFrames[i]--;
            }
            m_highlightAnmTransformFrames[i] = 0;
            m_highlightAnmColorFrames[i] = 0;
            m_nameAnmTevRegKeyFrames[i] = 0;
        }
    }
    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 35;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_selectAnmTransform->m_frame = m_selectAnmTransformFrame;
    m_okAnmTextureSRTKey->m_frame = m_okAnmTextureSRTKeyFrame;
    m_okAnmColor->m_frame = m_okAnmColorFrame;
    m_gridAnmTransform->m_frame = m_gridAnmTransformFrame;
    for (u32 i = 0; i < m_mapAnmTransforms.count(); i++) {
        m_mapAnmTransforms[i]->m_frame = m_mapAnmTransformFrames[i];
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
    m_nameAnmTransform->m_frame = m_nameAnmTransformFrame;
    for (u32 i = 0; i < m_nameAnmTevRegKeys.count(); i++) {
        m_nameAnmTevRegKeys[i]->m_frame = m_nameAnmTevRegKeyFrames[i];
    }
    m_arrowAnmTransform->m_frame = m_arrowAnmTransformFrame;

    for (u32 i = 0; i < m_mapAlphas.count(); i++) {
        m_mapScreens[i].search("MapPict")->setAlpha(m_mapAlphas[i]);
        m_mapScreens[i].search("WMap")->setAlpha(m_mapAlphas[i]);
        m_mapScreens[i].search("Name")->setAlpha(m_mapAlphas[i]);
        if (m_mapAlphas[i] != 255) {
            m_highlightAnmColors[i]->m_frame = (255 - m_mapAlphas[i]) * 59 / 255;
        }
    }
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_arrowScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }

    m_mainScreen.animation();
    m_gridScreen.animation();
    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        m_mapScreens[i].animationMaterials();
    }
    m_arrowScreen.animation();

    if (sequenceInfo.m_isOnline) {
        OnlineTimer::Instance()->calc();
    }
}

void SceneMapSelect::slideIn() {
    RaceInfo &raceInfo = RaceInfo::Instance();
    if (raceInfo.isRace()) {
        MenuTitleLine::Instance()->drop(MenuTitleLine::Title::SelectCourse);
    } else {
        MenuTitleLine::Instance()->drop(MenuTitleLine::Title::SelectMap);
    }
    m_mainAnmTransformFrame = 0;
    m_selectAnmTransformFrame = 0;
    if (m_mapIndex / 3 == m_rowIndex) {
        m_gridAnmTransformFrame = 11;
    } else {
        m_gridAnmTransformFrame = 10;
    }
    m_thumbnailAnmTevRegKeyFrames.fill(1);
    m_nameAnmTransformFrame = raceInfo.isRace() ? 0 : 6;
    for (u32 i = 0; i < m_mapAlphas.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        if (i < 6 && mapIndex < m_mapCount) {
            m_mapAlphas[i] = 255;
        } else {
            m_mapAlphas[i] = 0;
        }
    }
    m_arrowAlphas.fill(0);
    m_currMapIndices.fill(UINT32_MAX);
    OSInitMessageQueue(&m_queue, m_messages.values(), m_messages.count());
    u32 stackSize = 64 * 1024;
    m_loadStack.reset(new (m_heap, 0x8) u8[stackSize]);
    OSCreateThread(&m_loadThread, Load, this, m_loadStack.get() + stackSize, stackSize, 25, 0);
    OSResumeThread(&m_loadThread);
    refreshMaps();
    m_state = &SceneMapSelect::stateSlideIn;
}

void SceneMapSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    if (m_mapIndex / 3 == m_rowIndex) {
        m_gridAnmTransformFrame = 21;
    } else {
        m_gridAnmTransformFrame = 0;
    }
    m_state = &SceneMapSelect::stateSlideOut;
}

void SceneMapSelect::idle() {
    m_state = &SceneMapSelect::stateIdle;
}

void SceneMapSelect::moveUp() {
    m_mapIndex -= 3;
    m_gridAnmTransformFrame = 28;
    m_state = &SceneMapSelect::stateMoveUp;
}

void SceneMapSelect::moveDown() {
    m_mapIndex = Min(m_mapIndex + 3, m_mapCount - 1);
    m_gridAnmTransformFrame = 22;
    m_state = &SceneMapSelect::stateMoveDown;
}

void SceneMapSelect::scrollUp() {
    m_mapIndex -= 3;
    m_rowIndex--;
    m_gridAnmTransformFrame = 39;
    m_mapAnmTransformFrames.rotateRight(3);
    m_highlightAnmTransformFrames.rotateRight(3);
    m_highlightAnmColorFrames.rotateRight(3);
    m_thumbnailAnmTevRegKeyFrames.rotateRight(3);
    m_nameAnmTevRegKeyFrames.rotateRight(3);
    m_mapAlphas.rotateRight(3);
    refreshMaps();
    m_state = &SceneMapSelect::stateScrollUp;
}

void SceneMapSelect::scrollDown() {
    m_mapIndex = Min(m_mapIndex + 3, m_mapCount - 1);
    m_gridAnmTransformFrame = 30;
    m_state = &SceneMapSelect::stateScrollDown;
}

void SceneMapSelect::spin() {
    m_spinFrame = 1;
    refreshSpin();
    if (m_gridAnmTransformFrame > 21) {
        m_gridAnmTransformFrame--;
    }
    m_state = &SceneMapSelect::stateSpin;
}

void SceneMapSelect::selectIn() {
    m_selectAnmTransformFrame = 1;
    if (m_mapIndex / 3 == m_rowIndex) {
        if (m_gridAnmTransformFrame > 21) {
            m_gridAnmTransformFrame--;
        }
    } else {
        if (m_gridAnmTransformFrame < 29) {
            m_gridAnmTransformFrame++;
        }
    }
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeyFrames.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        m_thumbnailAnmTevRegKeyFrames[i] = mapIndex == m_mapIndex;
    }
    showMaps(0);
    m_state = &SceneMapSelect::stateSelectIn;
}

void SceneMapSelect::selectOut() {
    m_selectAnmTransformFrame = 8;
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeyFrames.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        if (mapIndex != m_mapIndex) {
            m_thumbnailAnmTevRegKeyFrames[i] = 1;
        }
    }
    m_state = &SceneMapSelect::stateSelectOut;
}

void SceneMapSelect::select() {
    m_state = &SceneMapSelect::stateSelect;
}

void SceneMapSelect::nextScene() {
    for (u32 i = 0; i < m_thumbnails.count(); i++) {
        m_thumbnails[i].reset();
    }
    for (u32 i = 0; i < m_nameImages.count(); i++) {
        m_nameImages[i].reset();
    }
    refreshMaps();
    m_state = &SceneMapSelect::stateNextScene;
}

void SceneMapSelect::nextBattle() {
    RaceInfo::Instance().m_vsLapNum = SystemRecord::Instance().m_vsLapNum;
    m_state = &SceneMapSelect::stateNextBattle;
}

void SceneMapSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        if (m_mainAnmTransformFrame <= 10) {
            if (m_mapIndex / 3 == m_rowIndex) {
                m_gridAnmTransformFrame = 11 + m_mainAnmTransformFrame;
            } else {
                m_gridAnmTransformFrame = 10 - m_mainAnmTransformFrame;
            }
        } else {
            showArrows(0);
        }
    } else {
        idle();
    }
}

void SceneMapSelect::stateSlideOut() {
    uintptr_t msg = false;
    OSSendMessage(&m_queue, reinterpret_cast<void *>(msg), OS_MESSAGE_NOBLOCK);
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_mainAnmTransformFrame >= 10) {
            hideArrows();
        } else {
            if (m_mapIndex / 3 == m_rowIndex) {
                m_gridAnmTransformFrame = 11 + m_mainAnmTransformFrame;
            } else {
                m_gridAnmTransformFrame = 10 - m_mainAnmTransformFrame;
            }
        }
        if (SequenceInfo::Instance().m_isOnline) {
            if (m_nextScene != SceneType::CharacterSelect) {
                OnlineTimer::Instance()->setAlpha(m_mainAnmTransformFrame * 17);
            }
        }
    } else {
        if (OSIsThreadTerminated(&m_loadThread)) {
            OSDetachThread(&m_loadThread);
            m_loadStack.reset();
            if (m_nextScene == SceneType::None) {
                nextBattle();
            } else {
                nextScene();
            }
        }
    }
}

void SceneMapSelect::stateIdle() {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A ||
            (sequenceInfo.m_isOnline && OnlineTimer::Instance()->hasExpired())) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        selectIn();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = sequenceInfo.m_isOnline ? SceneType::CharacterSelect : SceneType::PackSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_mapIndex / 3 >= 1) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_mapIndex / 3 == m_rowIndex) {
                scrollUp();
            } else {
                moveUp();
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_mapIndex / 3 + 1 < (m_mapCount + 3 - 1) / 3) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_mapIndex / 3 == m_rowIndex + 1) {
                scrollDown();
            } else {
                moveDown();
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        u32 colIndex = m_mapIndex % 3;
        colIndex = colIndex == 0 ? 2 : colIndex - 1;
        m_mapIndex = m_mapIndex / 3 * 3 + colIndex;
        m_mapIndex = Min(m_mapIndex, m_mapCount - 1);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        u32 colIndex = m_mapIndex % 3;
        colIndex = colIndex == 2 ? 0 : colIndex + 1;
        m_mapIndex = m_mapIndex / 3 * 3 + colIndex;
        m_mapIndex = Min(m_mapIndex, m_mapCount - 1);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
    } else if ((button.level() & PAD_TRIGGER_R) && (button.level() & PAD_TRIGGER_L)) {
        spin();
    }
}

void SceneMapSelect::stateMoveUp() {
    m_gridAnmTransformFrame--;
    if (m_gridAnmTransformFrame == 21) {
        idle();
    }
}

void SceneMapSelect::stateMoveDown() {
    m_gridAnmTransformFrame++;
    if (m_gridAnmTransformFrame == 29) {
        idle();
    }
}

void SceneMapSelect::stateScrollUp() {
    m_gridAnmTransformFrame++;
    showMaps(0);
    showArrows(0);
    if (m_gridAnmTransformFrame == 46) {
        m_gridAnmTransformFrame = 21;
        idle();
    }
}

void SceneMapSelect::stateScrollDown() {
    m_gridAnmTransformFrame++;
    showMaps(1);
    showArrows(1);
    if (m_gridAnmTransformFrame == 37) {
        m_rowIndex++;
        m_gridAnmTransformFrame = 29;
        m_mapAnmTransformFrames.rotateLeft(3);
        m_highlightAnmTransformFrames.rotateLeft(3);
        m_highlightAnmColorFrames.rotateLeft(3);
        m_thumbnailAnmTevRegKeyFrames.rotateLeft(3);
        m_nameAnmTevRegKeyFrames.rotateLeft(3);
        m_mapAlphas.rotateLeft(3);
        refreshMaps();
        idle();
    }
}

void SceneMapSelect::stateSpin() {
    if (m_gridAnmTransformFrame > 21) {
        m_gridAnmTransformFrame--;
    }
    hideArrows();
    m_spinFrame++;
    bool isSpinning = m_spinFrame < 30;
    if (!isSpinning) {
        const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
        isSpinning = button.level() & PAD_TRIGGER_R && button.level() & PAD_TRIGGER_L;
    }
    if (SequenceInfo::Instance().m_isOnline) {
        if (OnlineTimer::Instance()->hasExpired()) {
            isSpinning = false;
        }
    }
    if (isSpinning) {
        if (m_spinFrame % 5 == 0) {
            m_mapIndex = m_spinMapIndex;
            m_rowIndex = m_spinRowIndex;
            refreshSpin();
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
        showMaps(0);
    } else {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        selectIn();
    }
}

void SceneMapSelect::stateSelectIn() {
    m_selectAnmTransformFrame++;
    if (m_mapIndex / 3 == m_rowIndex) {
        if (m_gridAnmTransformFrame > 21) {
            m_gridAnmTransformFrame--;
        }
    } else {
        if (m_gridAnmTransformFrame < 29) {
            m_gridAnmTransformFrame++;
        }
    }
    showMaps(0);
    hideArrows();
    if (m_selectAnmTransformFrame == 9) {
        select();
    }
}

void SceneMapSelect::stateSelectOut() {
    m_selectAnmTransformFrame--;
    showArrows(0);
    if (m_selectAnmTransformFrame == 0) {
        idle();
    }
}

void SceneMapSelect::stateSelect() {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    CourseManager *courseManager = CourseManager::Instance();
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A ||
            (sequenceInfo.m_isOnline && OnlineTimer::Instance()->hasExpired())) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
        if (sequenceInfo.m_isOnline) {
            m_nextScene = SceneType::CoursePoll;
        } else {
            m_nextScene = SceneType::None;
            GameAudio::Main::Instance()->fadeOutAll(15);
            System::GetDisplay()->startFadeOut(15);
            const CourseManager::Course *course;
            if (RaceInfo::Instance().isRace()) {
                course = &courseManager->raceCourse(sequenceInfo.m_packIndex, m_mapIndex);
            } else {
                course = &courseManager->battleCourse(sequenceInfo.m_packIndex, m_mapIndex);
            }
            ResMgr::LoadExtendedCourseData(course, 2);
            SequenceInfo::Instance().m_mapIndex = m_mapIndex;
        }
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        selectOut();
    }
}

void SceneMapSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneMapSelect::refreshSpin() {
    CubeRandom *random = CubeRandom::Instance();
    m_spinMapIndex = random->get(m_mapCount);
    m_spinRowIndex = m_spinMapIndex / 3;
    if (m_spinRowIndex > 0) {
        m_spinRowIndex -= random->get(2);
    }
    if (m_rowCount >= 2 && m_spinRowIndex == m_rowCount - 1) {
        m_spinRowIndex--;
    }
    Array<u32, 12> mapIndices;
    for (u32 i = 0; i < 6; i++) {
        mapIndices[0 + i] = m_rowIndex * 3 + i;
    }
    for (u32 i = 0; i < 6; i++) {
        mapIndices[6 + i] = m_spinRowIndex * 3 + i;
    }
    refreshMaps(mapIndices);
}

void SceneMapSelect::refreshMaps() {
    Array<u32, 12> mapIndices;
    u32 rowIndex = m_rowIndex == 0 ? 0 : m_rowIndex - 1;
    for (u32 i = 0; i < mapIndices.count(); i++) {
        mapIndices[i] = rowIndex * 3 + i;
    }
    refreshMaps(mapIndices);
}

void SceneMapSelect::refreshMaps(const Array<u32, 12> &nextMapIndices) {
    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        Array<const char *, 2> names;
        names[0] = "MapPict";
        names[1] = "Name";
        for (u32 j = 0; j < names.count(); j++) {
            J2DPicture *picture = m_mapScreens[i].search(names[j])->downcast<J2DPicture>();
            picture->m_isVisible = false;
            picture->changeTexture("SelCourse_Pict_Box1.bti", 0);
        }
    }

    Lock<Mutex> m_lock(m_mutex);
    m_nextMapIndices = nextMapIndices;
    uintptr_t msg = true;
    OSSendMessage(&m_queue, reinterpret_cast<void *>(msg), OS_MESSAGE_NOBLOCK);
}

void SceneMapSelect::showMaps(s32 rowOffset) {
    for (u32 i = 0; i < m_mapAlphas.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        if (static_cast<s32>(i) >= rowOffset * 3 && static_cast<s32>(i) < 6 + rowOffset * 3 &&
                mapIndex < m_mapCount) {
            if (m_mapAlphas[i] < 255) {
                m_mapAlphas[i] += 51;
            }
        } else {
            if (m_mapAlphas[i] > 0) {
                m_mapAlphas[i] -= 51;
            }
        }
    }
}

void SceneMapSelect::showArrows(s32 rowOffset) {
    if (m_rowIndex + rowOffset > 0) {
        if (m_arrowAlphas[0] < 255) {
            m_arrowAlphas[0] += 51;
        }
    } else {
        if (m_arrowAlphas[0] > 0) {
            m_arrowAlphas[0] -= 51;
        }
    }
    if (m_rowIndex + rowOffset + 2 < m_rowCount) {
        if (m_arrowAlphas[1] < 255) {
            m_arrowAlphas[1] += 51;
        }
    } else {
        if (m_arrowAlphas[1] > 0) {
            m_arrowAlphas[1] -= 51;
        }
    }
}

void SceneMapSelect::hideArrows() {
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        if (m_arrowAlphas[i] > 0) {
            m_arrowAlphas[i] -= 51;
        }
    }
}

void *SceneMapSelect::load() {
    while (true) {
        uintptr_t msg;
        OSReceiveMessage(&m_queue, reinterpret_cast<void **>(&msg), OS_MESSAGE_BLOCK);
        if (!msg) {
            return nullptr;
        }

        while (true) {
            Array<u32, 12> nextMapIndices;
            {
                Lock<Mutex> lock(m_mutex);
                nextMapIndices = m_nextMapIndices;
            }

            if (load(nextMapIndices)) {
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

bool SceneMapSelect::load(const Array<u32, 12> &nextMapIndices) {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    CourseManager *courseManager = CourseManager::Instance();
    for (u32 i = 0; i < nextMapIndices.count(); i++) {
        u32 mapIndex = nextMapIndices[i];
        if (mapIndex >= m_mapCount) {
            continue;
        }
        const CourseManager::Course *course;
        if (RaceInfo::Instance().isRace()) {
            course = &courseManager->raceCourse(sequenceInfo.m_packIndex, mapIndex);
        } else {
            course = &courseManager->battleCourse(sequenceInfo.m_packIndex, mapIndex);
        }
        UniquePtr<ResTIMG> &thumbnail = findTexture(m_thumbnails, nextMapIndices, mapIndex);
        if (!thumbnail.get()) {
            void *texture = course->loadThumbnail(m_heap);
            {
                Lock<Mutex> lock(m_mutex);
                thumbnail.reset(static_cast<ResTIMG *>(texture));
            }
            return false;
        }
        UniquePtr<ResTIMG> &nameImage = findTexture(m_nameImages, nextMapIndices, mapIndex);
        if (!nameImage.get()) {
            void *texture = course->loadNameImage(m_heap);
            {
                Lock<Mutex> lock(m_mutex);
                nameImage.reset(static_cast<ResTIMG *>(texture));
            }
            return false;
        }
    }
    return true;
}

UniquePtr<ResTIMG> &SceneMapSelect::findTexture(Array<UniquePtr<ResTIMG>, 12> &textures,
        const Array<u32, 12> &nextMapIndices, u32 mapIndex) {
    for (u32 i = 0; i < m_currMapIndices.count(); i++) {
        if (m_currMapIndices[i] == mapIndex) {
            return textures[i];
        }
    }
    u32 i;
    for (i = 0; i < m_currMapIndices.count(); i++) {
        u32 j;
        for (j = 0; j < nextMapIndices.count(); j++) {
            if (nextMapIndices[j] == m_currMapIndices[i]) {
                break;
            }
        }
        if (j == nextMapIndices.count()) {
            break;
        }
    }
    assert(i < m_currMapIndices.count());
    Lock<Mutex> lock(m_mutex);
    m_thumbnails[i].reset();
    m_nameImages[i].reset();
    m_currMapIndices[i] = mapIndex;
    return textures[i];
}

void *SceneMapSelect::Load(void *param) {
    return static_cast<SceneMapSelect *>(param)->load();
}
