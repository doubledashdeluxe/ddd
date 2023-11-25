#include "SceneMapSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/ResMgr.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <common/Algorithm.hh>
extern "C" {
#include <dolphin/OSTime.h>
}
#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>

extern "C" {
#include <stdio.h>
}

SceneMapSelect::SceneMapSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *backgroundArchive = sceneFactory->archive(SceneFactory::ArchiveType::Background);
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);
    JKRArchive *ghostDataArchive = sceneFactory->archive(SceneFactory::ArchiveType::GhostData);

    MenuBackground::Create(backgroundArchive);
    MenuTitleLine::Create(titleLineArchive, heap);

    m_mainScreen.set("SelectMapLayout.blo", 0x1040000, m_archive);
    m_gridScreen.set("SelectMapGrid.blo", 0x1040000, m_archive);
    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        m_mapScreens[i].set("SelectMap1.blo", 0x1040000, m_archive);
    }
    m_arrowScreen.set("GDIndexLayout.blo", 0x20000, ghostDataArchive);

    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        Array<char, 9> tag;
        snprintf(tag.values(), tag.count(), "Map%u", i);
        m_gridScreen.search(tag.values())->appendChild(&m_mapScreens[i]);
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
    for (u32 i = 0; i < m_nameAnmTevRegKeys.count(); i++) {
        m_nameAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("SelectMap1.brk", m_archive);
        m_nameAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mapScreens[i]);
        m_mapScreens[i].search("Name")->setAnimation(m_nameAnmTevRegKeys[i]);
    }
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapArrow.bck", m_archive);
    for (u32 i = 0; i < 2; i++) {
        Array<char, 9> tag;
        snprintf(tag.values(), tag.count(), "MArrow%02u", i + 1);
        m_arrowScreen.search(tag.values())->setAnimation(m_arrowAnmTransform);
    }

    m_okAnmTextureSRTKeyFrame = 0;
    m_okAnmColorFrame = 0;
    m_mapAnmTransformFrames.fill(0);
    m_highlightAnmTransformFrames.fill(0);
    m_highlightAnmColorFrames.fill(0);
    m_nameAnmTevRegKeyFrames.fill(0);
    m_arrowAnmTransformFrame = 0;
}

SceneMapSelect::~SceneMapSelect() {}

void SceneMapSelect::init() {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    u32 battleMode = sequenceInfo.getBattleMode();
    u32 index = battleMode == 0 ? 0 : battleMode ^ 3;
    ResTIMG *texture = Kart2DCommon::Instance()->getBattleIcon(index);
    J2DPicture *picture = m_mainScreen.search("BtlPict")->downcast<J2DPicture>();
    picture->changeTexture(texture, 0);

    texture = BattleName2D::Instance()->getBattleNameTexture(battleMode);
    picture = m_mainScreen.search("SubM")->downcast<J2DPicture>();
    picture->changeTexture(texture, 0);

    m_mapCount = 6;
    m_mapIndex = 0;
    if (sequenceInfo.m_fromPause) {
        m_mapIndex = sequenceInfo.m_mapIndex;
        sequenceInfo.m_fromPause = false;
        System::GetDisplay()->startFadeIn(15);
        if (GameAudio::Main::Instance()->getPlayingSequenceID() != SoundID::JA_BGM_SELECT) {
            GameAudio::Main::Instance()->startSequenceBgm(SoundID::JA_BGM_SELECT);
        }
        SequenceApp::Instance()->ready(SceneType::PackSelect);
    }
    m_rowCount = (m_mapCount + 3 - 1) / 3;
    m_rowIndex = m_mapIndex / 3;
    m_rowIndex = Min(m_rowIndex, m_rowCount - Min<u32>(m_rowCount, 2));

    slideIn();
}

void SceneMapSelect::draw() {
    m_graphContext->setViewport();

    MenuBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_gridScreen.draw(0.0f, 0.0f, m_graphContext);
    m_arrowScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneMapSelect::calc() {
    (this->*m_state)();

    MenuBackground::Instance()->calc();
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
        Array<char, 9> tag;
        snprintf(tag.values(), tag.count(), "MArrow%02u", i + 1);
        m_arrowScreen.search(tag.values())->setAlpha(m_arrowAlphas[i]);
    }

    m_mainScreen.animation();
    m_gridScreen.animation();
    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        m_mapScreens[i].animationMaterials();
    }
    m_arrowScreen.animation();
}

void SceneMapSelect::slideIn() {
    MenuTitleLine::Instance()->drop(MenuTitleLine::Title::SelectMap);
    m_mainAnmTransformFrame = 0;
    m_selectAnmTransformFrame = 0;
    if (m_mapIndex / 3 == m_rowIndex) {
        m_gridAnmTransformFrame = 11;
    } else {
        m_gridAnmTransformFrame = 10;
    }
    m_thumbnailAnmTevRegKeyFrames.fill(1);
    for (u32 i = 0; i < m_mapAlphas.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        if (i < 6 && mapIndex < m_mapCount) {
            m_mapAlphas[i] = 255;
        } else {
            m_mapAlphas[i] = 0;
        }
    }
    m_arrowAlphas.fill(0);
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
    m_mapIndex += 3;
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
    m_mapIndex += 3;
    m_gridAnmTransformFrame = 30;
    m_state = &SceneMapSelect::stateScrollDown;
}

void SceneMapSelect::selectIn() {
    m_selectAnmTransformFrame = 1;
    if (m_gridAnmTransformFrame == 25) {
        if (m_mapIndex / 3 == m_rowIndex) {
            m_gridAnmTransformFrame = 24;
        } else {
            m_gridAnmTransformFrame = 26;
        }
    }
    for (u32 i = 0; i < m_thumbnailAnmTevRegKeyFrames.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        if (mapIndex != m_mapIndex) {
            m_thumbnailAnmTevRegKeyFrames[i] = 0;
        }
    }
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

void SceneMapSelect::spin() {
    m_spinFrame = 1;
    if (m_mapIndex / 3 == m_rowIndex) {
        m_gridAnmTransformFrame = 22;
    } else {
        m_gridAnmTransformFrame = 28;
    }
    m_state = &SceneMapSelect::stateSpin;
}

void SceneMapSelect::nextScene() {
    m_state = &SceneMapSelect::stateNextScene;
}

void SceneMapSelect::nextBattle() {
    m_state = &SceneMapSelect::stateNextBattle;
}

void SceneMapSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        if (m_mapIndex / 3 == m_rowIndex) {
            if (m_gridAnmTransformFrame < 21) {
                m_gridAnmTransformFrame++;
            }
        } else {
            if (m_gridAnmTransformFrame > 0) {
                m_gridAnmTransformFrame--;
            }
        }
        if (m_mainAnmTransformFrame > 10) {
            showArrows(0);
        }
    } else {
        idle();
    }
}

void SceneMapSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_mainAnmTransformFrame < 10) {
            if (m_mapIndex / 3 == m_rowIndex) {
                if (m_gridAnmTransformFrame > 11) {
                    m_gridAnmTransformFrame--;
                }
            } else {
                if (m_gridAnmTransformFrame < 10) {
                    m_gridAnmTransformFrame++;
                }
            }
        }
        hideArrows();
    } else {
        if (m_nextScene == SceneType::None) {
            nextBattle();
        } else {
            nextScene();
        }
    }
}

void SceneMapSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        selectIn();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        m_nextScene = SceneType::PackSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_mapIndex >= 3) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_mapIndex / 3 == m_rowIndex) {
                scrollUp();
            } else {
                moveUp();
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_mapIndex + 3 < m_mapCount) {
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

void SceneMapSelect::stateSelectIn() {
    m_selectAnmTransformFrame++;
    if (m_gridAnmTransformFrame < 25 && m_gridAnmTransformFrame > 21) {
        m_gridAnmTransformFrame--;
    } else if (m_gridAnmTransformFrame > 25 && m_gridAnmTransformFrame < 29) {
        m_gridAnmTransformFrame++;
    }
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
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        m_nextScene = SceneType::None;
        GameAudio::Main::Instance()->fadeOutAll(15);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
        System::GetDisplay()->startFadeOut(15);
        u32 courseIds[] = {0x3a, 0x36, 0x35, 0x3b, 0x34, 0x38};
        ResMgr::LoadCourseData(courseIds[m_mapIndex % 6], 2);
        SequenceInfo::Instance().m_mapIndex = m_mapIndex;
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        selectOut();
    }
}

void SceneMapSelect::stateSpin() {
    if (m_gridAnmTransformFrame < 25) {
        m_gridAnmTransformFrame++;
    } else if (m_gridAnmTransformFrame > 25) {
        m_gridAnmTransformFrame--;
    }
    hideArrows();
    m_spinFrame++;
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (((button.level() & PAD_TRIGGER_R) && (button.level() & PAD_TRIGGER_L)) ||
            m_spinFrame < 30) {
        if (m_spinFrame % 5 == 0) {
            m_mapIndex = OSGetTime() % m_mapCount;
            m_rowIndex = m_mapIndex / 3;
            if (m_rowIndex > 0) {
                m_rowIndex -= OSGetTime() % 2;
            }
            if (m_rowCount >= 2 && m_rowIndex == m_rowCount - 1) {
                m_rowIndex--;
            }
            refreshMaps();
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        }
    } else {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_RANDOM_KETTEI);
        selectIn();
    }
}

void SceneMapSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneMapSelect::refreshMaps() {
    for (u32 i = 0; i < m_mapScreens.count(); i++) {
        u32 mapIndex = m_rowIndex * 3 + i;
        Array<char, 32> texture;
        snprintf(texture.values(), texture.count(), "BattleMapSnap%u.bti", mapIndex % 6 + 1);
        J2DPicture *picture = m_mapScreens[i].search("MapPict")->downcast<J2DPicture>();
        picture->changeTexture(texture.values(), 0);
        snprintf(texture.values(), texture.count(), "Mozi_Map%u.bti", mapIndex % 6 + 1);
        picture = m_mapScreens[i].search("Name")->downcast<J2DPicture>();
        picture->changeTexture(texture.values(), 0);
    }
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
