#include "ScenePackSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <common/Algorithm.hh>
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>

extern "C" {
#include <stdio.h>
}

ScenePackSelect::ScenePackSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *backgroundArchive = sceneFactory->archive(SceneFactory::ArchiveType::Background);
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);
    JKRArchive *mapSelectArchive = sceneFactory->archive(SceneFactory::ArchiveType::MapSelect);
    JKRArchive *ghostDataArchive = sceneFactory->archive(SceneFactory::ArchiveType::GhostData);

    MenuBackground::Create(backgroundArchive);
    MenuTitleLine::Create(titleLineArchive, heap);

    m_mainScreen.set("SelectPack.blo", 0x1040000, m_archive);
    m_modeScreen.set("SelectMapLayout.blo", 0x1040000, mapSelectArchive);
    for (u32 i = 0; i < m_countScreens.count(); i++) {
        m_countScreens[i].set("SelectPackCount.blo", 0x1040000, m_archive);
    }
    m_arrowScreen.set("GDIndexLayout.blo", 0x20000, ghostDataArchive);

    for (u32 i = 0; i < m_countScreens.count(); i++) {
        m_mainScreen.search("Ecount%u", i + 1)->appendChild(&m_countScreens[i]);
    }
    m_modeScreen.search("OK_wb11")->m_isVisible = false;
    m_arrowScreen.search("NSaveGD")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectPack.bck", m_archive);
    m_mainScreen.search("N_Entry")->setAnimation(m_mainAnmTransform);
    m_mainAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("HowManyDrivers.btk", m_archive);
    m_mainAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.setAnimation(m_mainAnmTextureSRTKey);
    for (u32 i = 0; i < m_packAnmTransforms.count(); i++) {
        m_packAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectPack.bck", m_archive);
        m_mainScreen.search("Ecrsr%u", i + 1)->setAnimation(m_packAnmTransforms[i]);
        m_mainScreen.search("ENplay%u", i + 1)->setAnimation(m_packAnmTransforms[i]);
        m_mainScreen.search("Ecount%u", i + 1)->setAnimation(m_packAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_packAnmColors.count(); i++) {
        m_packAnmColors[i] = J2DAnmLoaderDataBase::Load("SelectPack.bpk", m_archive);
        m_packAnmColors[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("Ecrsr%u", i + 1)->setAnimation(m_packAnmColors[i]);
    }
    for (u32 i = 0; i < m_packAnmTevRegKeys.count(); i++) {
        m_packAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("HowManyDrivers.brk", m_archive);
        m_packAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("Ebar%u", i + 1)->setAnimation(m_packAnmTevRegKeys[i]);
        m_mainScreen.search("Eplay%u", i + 1)->setAnimation(m_packAnmTevRegKeys[i]);
        m_mainScreen.search("Eplay%ub", i + 1)->setAnimation(m_packAnmTevRegKeys[i]);
        m_countScreens[i].setAnimation(m_packAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectPack.bck", m_archive);
        m_mainScreen.search("Eplay%ub", i + 1)->setAnimation(m_circleAnmTransforms[i]);
    }
    m_modeAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", mapSelectArchive);
    m_modeScreen.search("NSlMap")->setAnimation(m_modeAnmTransform);
    for (u32 i = 0; i < m_countAnmTransforms.count(); i++) {
        m_countAnmTransforms[i] = J2DAnmLoaderDataBase::Load("SelectPackCount.bck", m_archive);
        m_countScreens[i].setAnimation(m_countAnmTransforms[i]);
    }
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("GDIndexLayout.bck", ghostDataArchive);
    for (u32 i = 0; i < 2; i++) {
        m_arrowScreen.search("MArrow%02u", i + 1)->setAnimation(m_arrowAnmTransform);
    }

    m_mainAnmTextureSRTKeyFrame = 0;
    m_packAnmTransformFrames.fill(14);
    m_packAnmColorFrames.fill(0);
    m_packAnmTevRegKeyFrames.fill(0);
    m_circleAnmTransformFrames.fill(14);
    m_arrowAnmTransformFrame = 0;
}

ScenePackSelect::~ScenePackSelect() {}

void ScenePackSelect::init() {
    J2DPicture *iconPicture = m_modeScreen.search("BtlPict")->downcast<J2DPicture>();
    J2DPicture *namePicture = m_modeScreen.search("SubM")->downcast<J2DPicture>();
    RaceInfo &raceInfo = RaceInfo::Instance();
    switch (raceInfo.getRaceMode()) {
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

    if (CourseManager::Instance()->lock()) {
        slideIn();
    } else {
        wait();
    }
}

void ScenePackSelect::draw() {
    m_graphContext->setViewport();

    MenuBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_modeScreen.draw(0.0f, 0.0f, m_graphContext);
    m_arrowScreen.draw(0.0f, 0.0f, m_graphContext);
}

void ScenePackSelect::calc() {
    (this->*m_state)();

    MenuBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_mainAnmTextureSRTKeyFrame = (m_mainAnmTextureSRTKeyFrame + 1) % 180;
    for (u32 i = 0; i < 4; i++) {
        u32 packIndex = m_rowIndex + i;
        if (packIndex == m_packIndex) {
            if (m_packAnmTransformFrames[i] < 22) {
                m_packAnmTransformFrames[i]++;
            }
            m_packAnmTevRegKeyFrames[i] = 1;
            m_circleAnmTransformFrames[i] = 14 + (m_circleAnmTransformFrames[i] - 13) % 61;
        } else {
            if (m_packAnmTransformFrames[i] > 14) {
                m_packAnmTransformFrames[i]--;
            }
            m_packAnmTevRegKeyFrames[i] = 0;
            m_circleAnmTransformFrames[i] = 14;
        }
        m_packAnmColorFrames[i] = (m_packAnmColorFrames[i] + 1) % 120;
    }
    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 35;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_mainAnmTextureSRTKey->m_frame = m_mainAnmTextureSRTKeyFrame;
    for (u32 i = 0; i < m_packAnmTransforms.count(); i++) {
        m_packAnmTransforms[i]->m_frame = m_packAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_packAnmColors.count(); i++) {
        m_packAnmColors[i]->m_frame = m_packAnmColorFrames[i];
    }
    for (u32 i = 0; i < m_packAnmTevRegKeys.count(); i++) {
        m_packAnmTevRegKeys[i]->m_frame = m_packAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_circleAnmTransforms.count(); i++) {
        m_circleAnmTransforms[i]->m_frame = m_circleAnmTransformFrames[i];
    }
    m_modeAnmTransform->m_frame = m_modeAnmTransformFrame;
    m_arrowAnmTransform->m_frame = m_arrowAnmTransformFrame;

    for (u32 i = 0; i < m_packAlphas.count(); i++) {
        m_mainScreen.search("Ebar%u", i + 1)->setAlpha(m_packAlphas[i]);
        m_mainScreen.search("Eplay%u", i + 1)->setAlpha(m_packAlphas[i]);
        m_mainScreen.search("Eplay%ub", i + 1)->setAlpha(m_packAlphas[i]);
        for (u32 j = 0; j < 3; j++) {
            m_countScreens[i].search("Eplay%u", j + 1)->setAlpha(m_packAlphas[i]);
        }
        m_packAnmColors[i]->m_frame += m_packAlphas[i] / 51 * 121;
    }
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_arrowScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }

    m_mainScreen.animation();
    m_modeScreen.animation();
    for (u32 i = 0; i < m_countScreens.count(); i++) {
        m_countScreens[i].animationMaterials();
    }
    m_arrowScreen.animation();
}

void ScenePackSelect::wait() {
    m_state = &ScenePackSelect::stateWait;
}

void ScenePackSelect::slideIn() {
    if (RaceInfo::Instance().isRace()) {
        m_packCount = CourseManager::Instance()->racePackCount();
    } else {
        m_packCount = CourseManager::Instance()->battlePackCount();
    }
    m_packIndex = 0;
    if (SequenceApp::Instance()->prevScene() == SceneType::MapSelect) {
        m_packIndex = SequenceInfo::Instance().m_packIndex;
        SequenceApp::Instance()->ready(SceneType::Menu);
    }
    m_rowIndex = m_packIndex;
    m_rowIndex = Min(m_rowIndex, m_packCount - Min<u32>(m_packCount, 3));

    MenuTitleLine::Instance()->drop("SelectPack.bti");
    m_modeAnmTransformFrame = 0;
    m_mainAnmTransformFrame = 0;
    for (u32 i = 0; i < m_packAlphas.count(); i++) {
        u32 packIndex = m_rowIndex + i;
        if (i < 3 && packIndex < m_packCount) {
            m_packAlphas[i] = 255;
        } else {
            m_packAlphas[i] = 0;
        }
    }
    m_arrowAlphas.fill(0);
    refreshPacks();
    m_state = &ScenePackSelect::stateSlideIn;
}

void ScenePackSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &ScenePackSelect::stateSlideOut;
}

void ScenePackSelect::idle() {
    m_state = &ScenePackSelect::stateIdle;
}

void ScenePackSelect::scrollUp() {
    m_packIndex--;
    m_rowIndex--;
    m_mainAnmTransformFrame = 21;
    m_packAnmTransformFrames.rotateRight(1);
    m_packAnmTevRegKeyFrames.rotateRight(1);
    m_circleAnmTransformFrames.rotateRight(1);
    m_packAlphas.rotateRight(1);
    refreshPacks();
    m_state = &ScenePackSelect::stateScrollUp;
}

void ScenePackSelect::scrollDown() {
    m_packIndex++;
    m_mainAnmTransformFrame = 15;
    m_state = &ScenePackSelect::stateScrollDown;
}

void ScenePackSelect::nextScene() {
    m_state = &ScenePackSelect::stateNextScene;
}

void ScenePackSelect::stateWait() {
    if (CourseManager::Instance()->lock()) {
        slideIn();
    }
}

void ScenePackSelect::stateSlideIn() {
    if (m_modeAnmTransformFrame < 15) {
        m_modeAnmTransformFrame++;
        if (m_modeAnmTransformFrame <= 14) {
            m_mainAnmTransformFrame = m_modeAnmTransformFrame;
        }
        if (m_modeAnmTransformFrame > 10) {
            showArrows(0);
        }
    } else {
        idle();
    }
}

void ScenePackSelect::stateSlideOut() {
    if (m_modeAnmTransformFrame > 0) {
        m_modeAnmTransformFrame--;
        if (m_modeAnmTransformFrame <= 14) {
            m_mainAnmTransformFrame = m_modeAnmTransformFrame;
        }
        hideArrows();
    } else {
        nextScene();
    }
}

void ScenePackSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::MapSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        SequenceInfo::Instance().m_packIndex = m_packIndex;
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::Menu;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_packIndex >= 1) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_packIndex == m_rowIndex) {
                scrollUp();
            } else {
                m_packIndex--;
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_packIndex + 1 < m_packCount) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_packIndex == m_rowIndex + 2) {
                scrollDown();
            } else {
                m_packIndex++;
            }
        }
    }
}

void ScenePackSelect::stateScrollUp() {
    m_mainAnmTransformFrame--;
    showPacks(0);
    showArrows(0);
    if (m_mainAnmTransformFrame == 14) {
        idle();
    }
}

void ScenePackSelect::stateScrollDown() {
    m_mainAnmTransformFrame++;
    showPacks(1);
    showArrows(1);
    if (m_mainAnmTransformFrame == 22) {
        m_rowIndex++;
        m_mainAnmTransformFrame = 14;
        m_packAnmTransformFrames.rotateLeft(1);
        m_packAnmTevRegKeyFrames.rotateLeft(1);
        m_circleAnmTransformFrames.rotateLeft(1);
        m_packAlphas.rotateLeft(1);
        refreshPacks();
        idle();
    }
}

void ScenePackSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void ScenePackSelect::refreshPacks() {
    CourseManager *courseManager = CourseManager::Instance();
    for (u32 i = 0; i < 4; i++) {
        u32 packIndex = m_rowIndex + i;
        if (packIndex >= m_packCount) {
            break;
        }
        const CourseManager::Pack *pack;
        if (RaceInfo::Instance().isRace()) {
            pack = &courseManager->racePack(packIndex);
        } else {
            pack = &courseManager->battlePack(packIndex);
        }
        J2DPicture *picture = m_mainScreen.search("Eplay%u", i + 1)->downcast<J2DPicture>();
        picture->changeTexture(reinterpret_cast<ResTIMG *>(pack->nameImage()), 0);
        Array<J2DPicture *, 3> pictures;
        pictures[0] = m_countScreens[i].search("Eplay3")->downcast<J2DPicture>();
        pictures[1] = m_countScreens[i].search("Eplay2")->downcast<J2DPicture>();
        pictures[2] = m_countScreens[i].search("Eplay1")->downcast<J2DPicture>();
        Kart2DCommon::Instance()->changeNumberTexture(pack->courseIndices().count(),
                pictures.values(), pictures.count(), false, false);
    }
}

void ScenePackSelect::showPacks(s32 rowOffset) {
    for (u32 i = 0; i < m_packAlphas.count(); i++) {
        u32 packIndex = m_rowIndex + i;
        if (static_cast<s32>(i) >= rowOffset && static_cast<s32>(i) < 3 + rowOffset &&
                packIndex < m_packCount) {
            if (m_packAlphas[i] < 255) {
                m_packAlphas[i] += 51;
            }
        } else {
            if (m_packAlphas[i] > 0) {
                m_packAlphas[i] -= 51;
            }
        }
    }
}

void ScenePackSelect::showArrows(s32 rowOffset) {
    if (m_rowIndex + rowOffset > 0) {
        if (m_arrowAlphas[0] < 255) {
            m_arrowAlphas[0] += 51;
        }
    } else {
        if (m_arrowAlphas[0] > 0) {
            m_arrowAlphas[0] -= 51;
        }
    }
    if (m_rowIndex + rowOffset + 3 < m_packCount) {
        if (m_arrowAlphas[1] < 255) {
            m_arrowAlphas[1] += 51;
        }
    } else {
        if (m_arrowAlphas[1] > 0) {
            m_arrowAlphas[1] -= 51;
        }
    }
}

void ScenePackSelect::hideArrows() {
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        if (m_arrowAlphas[i] > 0) {
            m_arrowAlphas[i] -= 51;
        }
    }
}
