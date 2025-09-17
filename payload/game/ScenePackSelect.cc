#include "ScenePackSelect.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuBackground.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/RoomType.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/CourseManager.hh>
#include <portable/Algorithm.hh>
#include <portable/UTF8.hh>

ScenePackSelect::ScenePackSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *mapSelectArchive = sceneFactory->archive(SceneFactory::ArchiveType::MapSelect);
    JKRArchive *backgroundArchive = sceneFactory->archive(SceneFactory::ArchiveType::Background);
    JKRArchive *titleLineArchive = sceneFactory->archive(SceneFactory::ArchiveType::TitleLine);

    if (!SequenceInfo::Instance().m_isOnline) {
        MenuBackground::Create(backgroundArchive);
    }
    MenuTitleLine::Create(titleLineArchive, heap);

    m_mainScreen.set("GDIndexLayout.blo", 0x20000, m_archive);
    m_modeScreen.set("SelectMapLayout.blo", 0x1040000, mapSelectArchive);
    for (u32 i = 0; i < m_packScreens.count(); i++) {
        m_packScreens[i].set("Line.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_packScreens.count(); i++) {
        m_packScreens[i].search("GDCurs")->setHasARTrans(false, true);
        m_packScreens[i].search("GDCurs")->setHasARShift(false, true);
        m_packScreens[i].search("GDCurs")->setHasARScale(false, true);
    }

    for (u32 i = 0; i < m_packScreens.count(); i++) {
        m_mainScreen.search("Num%02u", i)->appendChild(&m_packScreens[i]);
    }
    m_mainScreen.search("NSaveGD")->m_isVisible = false;
    m_modeScreen.search("OK_wb11")->m_isVisible = false;
    m_modeScreen.search("NRandom")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectPackLayout.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_modeAnmTransform = J2DAnmLoaderDataBase::Load("SelectMapLayout.bck", mapSelectArchive);
    m_modeScreen.search("NSlMap")->setAnimation(m_modeAnmTransform);
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("SelectPackLayout.bck", m_archive);
    for (u32 i = 0; i < 2; i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAnimation(m_arrowAnmTransform);
    }
    m_onlineAnmTransform = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
    for (u32 i = 0; i < 6; i++) {
        m_packScreens[i].search("CIcon")->setAnimation(m_onlineAnmTransform);
        m_packScreens[i].search("CCount")->setAnimation(m_onlineAnmTransform);
    }
    for (u32 i = 0; i < m_packAnmTransforms.count(); i++) {
        m_packAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_packScreens[i].setAnimation(m_packAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_descAnmTransforms.count(); i++) {
        m_descAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_packScreens[i].search("Desc")->setAnimation(m_descAnmTransforms[i]);
    }

    m_arrowAnmTransformFrame = 0;
    m_packAnmTransformFrames.fill(0);
    m_descAnmTransformFrames.fill(0);
    m_arrowAlphas.fill(0);
    m_packAlphas.fill(0);
    m_descAlphas.fill(0);
}

ScenePackSelect::~ScenePackSelect() {}

void ScenePackSelect::init() {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    for (u32 i = 0; i < m_packScreens.count(); i++) {
        m_packScreens[i].search("PIcon")->m_isVisible = sequenceInfo.m_isOnline;
        m_packScreens[i].search("PCount")->m_isVisible = sequenceInfo.m_isOnline;
    }

    J2DPicture *iconPicture = m_modeScreen.search("BtlPict")->downcast<J2DPicture>();
    J2DPicture *namePicture = m_modeScreen.search("SubM")->downcast<J2DPicture>();
    RaceInfo &raceInfo = RaceInfo::Instance();
    const char *iconTextureName = RaceMode::IconTextureName(raceInfo.m_raceMode);
    iconPicture->changeTexture(iconTextureName, 0);
    const char *nameTextureName = RaceMode::NameTextureName(raceInfo.m_raceMode);
    namePicture->changeTexture(nameTextureName, 0);
    for (u32 i = 0; i < m_packScreens.count(); i++) {
        J2DPicture *picture = m_packScreens[i].search("CIcon")->downcast<J2DPicture>();
        if (RaceInfo::Instance().isRace()) {
            picture->changeTexture("Course.bti", 0);
        } else {
            picture->changeTexture("Arena.bti", 0);
        }
    }

    m_packCount = 0;

    if (CourseManager::Instance()->lock()) {
        slideIn();
    } else {
        wait();
    }
}

void ScenePackSelect::draw() {
    m_graphContext->setViewport();

    if (SequenceInfo::Instance().m_isOnline) {
        OnlineBackground::Instance()->draw(m_graphContext);
    } else {
        MenuBackground::Instance()->draw(m_graphContext);
    }
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_modeScreen.draw(0.0f, 0.0f, m_graphContext);
}

void ScenePackSelect::calc() {
    (this->*m_state)();

    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    if (sequenceInfo.m_isOnline) {
        OnlineBackground::Instance()->calc();
    } else {
        MenuBackground::Instance()->calc();
    }
    MenuTitleLine::Instance()->calc();

    m_descOffset += 5;
    refreshPacks();

    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 35;
    m_onlineAnmTransformFrame = sequenceInfo.m_isOnline * 12;
    for (u32 i = 0; i < 6; i++) {
        u32 packIndex = m_rowIndex + i;
        if (packIndex == m_packIndex) {
            if (m_packAnmTransformFrames[i] < 7) {
                m_packAnmTransformFrames[i]++;
            }
        } else {
            if (m_packAnmTransformFrames[i] > 0) {
                m_packAnmTransformFrames[i]--;
            }
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_modeAnmTransform->m_frame = m_modeAnmTransformFrame;
    m_arrowAnmTransform->m_frame = m_arrowAnmTransformFrame;
    m_onlineAnmTransform->m_frame = m_onlineAnmTransformFrame;
    for (u32 i = 0; i < m_packAnmTransforms.count(); i++) {
        m_packAnmTransforms[i]->m_frame = m_packAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_descAnmTransforms.count(); i++) {
        m_descAnmTransforms[i]->m_frame = m_descAnmTransformFrames[i];
    }

    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }
    for (u32 i = 0; i < m_packAlphas.count(); i++) {
        m_packScreens[i].search("GDCurs")->setAlpha(m_packAlphas[i]);
        m_packScreens[i].search("GDCurs1")->setAlpha(m_packAlphas[i]);
        for (s32 j = 0; j < 26; j++) {
            u8 alpha = m_packAlphas[i];
            if (j > 25 - sequenceInfo.m_isOnline * 4) {
                alpha = 0;
            }
            m_packScreens[i].search("Name%u", j)->setAlpha(alpha);
        }
        for (s32 j = 0; j < 42; j++) {
            u8 alpha = m_packAlphas[i];
            if (j == 0) {
                alpha = (alpha * (255 - m_descAlphas[i])) >> 8;
            } else if (j == 41 - sequenceInfo.m_isOnline * 6) {
                alpha = (alpha * m_descAlphas[i]) >> 8;
            } else if (j > 41 - sequenceInfo.m_isOnline * 6) {
                alpha = 0;
            }
            m_packScreens[i].search("Desc%u", j)->setAlpha(alpha);
        }
        for (u32 j = 0; j < 2; j++) {
            u8 alpha = m_packAlphas[i];
            m_packScreens[i].search("%cIcon", "CP"[j])->setAlpha(alpha);
            for (u32 k = 1; k <= 3; k++) {
                for (u32 l = 0; l < k; l++) {
                    m_packScreens[i].search("%cCount%u%u", "CP"[j], k, l)->setAlpha(alpha);
                }
            }
        }
    }

    m_mainScreen.animation();
    m_modeScreen.animation();
    for (u32 i = 0; i < m_packScreens.count(); i++) {
        m_packScreens[i].animationMaterials();
    }
}

ScenePackSelect::DescText::DescText(ScenePackSelect &scene, u32 descIndex)
    : m_scene(scene), m_descIndex(descIndex) {}

ScenePackSelect::DescText::~DescText() {}

const char *ScenePackSelect::DescText::getPart(u32 partIndex) {
    CourseManager *courseManager = CourseManager::Instance();
    u32 packIndex = m_scene.m_rowIndex + m_descIndex;
    const CourseManager::Course *course;
    if (RaceInfo::Instance().isRace()) {
        course = &courseManager->raceCourse(packIndex, partIndex);
    } else {
        course = &courseManager->battleCourse(packIndex, partIndex);
    }
    return course->name();
}

void ScenePackSelect::DescText::setAnmTransformFrame(u8 anmTransformFrame) {
    m_scene.m_descAnmTransformFrames[m_descIndex] = anmTransformFrame;
}

void ScenePackSelect::DescText::setAlpha(u8 alpha) {
    m_scene.m_descAlphas[m_descIndex] = alpha;
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
    s32 prevScene = SequenceApp::Instance()->prevScene();
    if (SequenceInfo::Instance().m_isOnline) {
        if (prevScene != SceneType::RoomTypeSelect && prevScene != SceneType::ModeSelect) {
            m_packIndex = SequenceInfo::Instance().m_packIndex;
        }
    } else {
        if (prevScene == SceneType::MapSelect) {
            m_packIndex = SequenceInfo::Instance().m_packIndex;
            SequenceApp::Instance()->ready(SceneType::Menu);
        }
    }
    m_rowIndex = m_packIndex;
    m_rowIndex = Min(m_rowIndex, m_packCount - Min<u32>(m_packCount, 5));
    m_descOffset = 0;

    MenuTitleLine::Instance()->drop("SelectPack.bti");
    m_mainAnmTransformFrame = 0;
    m_modeAnmTransformFrame = 0;
    for (u32 i = 0; i < m_packAlphas.count(); i++) {
        u32 packIndex = m_rowIndex + i;
        if (i < 5 && packIndex < m_packCount) {
            m_packAlphas[i] = 255;
        } else {
            m_packAlphas[i] = 0;
        }
    }
    m_state = &ScenePackSelect::stateSlideIn;
}

void ScenePackSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_mainAnmTransformFrame = 30;
    m_state = &ScenePackSelect::stateSlideOut;
}

void ScenePackSelect::idle() {
    m_state = &ScenePackSelect::stateIdle;
}

void ScenePackSelect::scrollUp() {
    m_packIndex--;
    m_rowIndex--;
    m_mainAnmTransformFrame = 46;
    m_packAnmTransformFrames.rotateRight(1);
    m_packAlphas.rotateRight(1);
    m_state = &ScenePackSelect::stateScrollUp;
}

void ScenePackSelect::scrollDown() {
    m_packIndex++;
    m_mainAnmTransformFrame = 40;
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
    if (m_mainAnmTransformFrame < 30) {
        m_mainAnmTransformFrame++;
        m_modeAnmTransformFrame = Min<u32>(m_mainAnmTransformFrame, 15);
        if (m_mainAnmTransformFrame > 25) {
            showArrows(0);
        }
    } else {
        idle();
    }
}

void ScenePackSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        m_modeAnmTransformFrame = Min<u32>(m_mainAnmTransformFrame, 15);
        hideArrows();
    } else {
        nextScene();
    }
}

void ScenePackSelect::stateIdle() {
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        if (sequenceInfo.m_isOnline) {
            switch (onlineInfo.m_roomType) {
            case RoomType::Worldwide:
                m_nextScene = SceneType::FormatSelect;
                break;
            case RoomType::Duel:
                m_nextScene = SceneType::PlayerList;
                System::GetDisplay()->startFadeOut(15);
                break;
            default:
                m_nextScene = SceneType::PersonalRoom;
                break;
            }
        } else {
            m_nextScene = SceneType::MapSelect;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        sequenceInfo.m_packIndex = m_packIndex;
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        if (sequenceInfo.m_isOnline) {
            if (onlineInfo.m_roomType == RoomType::Duel) {
                m_nextScene = SceneType::RoomTypeSelect;
            } else {
                m_nextScene = SceneType::ModeSelect;
            }
        } else {
            m_nextScene = SceneType::Menu;
        }
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
            if (m_packIndex == m_rowIndex + 4) {
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
    if (m_mainAnmTransformFrame == 39) {
        idle();
    }
}

void ScenePackSelect::stateScrollDown() {
    m_mainAnmTransformFrame++;
    showPacks(1);
    showArrows(1);
    if (m_mainAnmTransformFrame == 47) {
        m_rowIndex++;
        m_mainAnmTransformFrame = 39;
        m_packAnmTransformFrames.rotateLeft(1);
        m_packAlphas.rotateLeft(1);
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
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    CourseManager *courseManager = CourseManager::Instance();
    SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    for (u32 i = 0; i < 6; i++) {
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
        u32 namePictureCount = 26 - sequenceInfo.m_isOnline * 4;
        J2DScreen &screen = m_packScreens[i];
        kart2DCommon->changeUnicodeTexture(pack->name(), namePictureCount, screen, "Name");
        DescText descText(*this, i);
        u64 descOffset = Max<u64>(m_descOffset, 300) - 300;
        u32 courseCount = pack->courseIndices().count();
        u32 descPictureCount = 42 - sequenceInfo.m_isOnline * 6;
        descText.refresh(descOffset, courseCount, descPictureCount, screen, "Desc");
        kart2DCommon->changeNumberTexture(courseCount, 3, screen, "CCount");
        kart2DCommon->changeNumberTexture(packIndex * 12, 3, screen, "PCount");
    }
}

void ScenePackSelect::showPacks(s32 rowOffset) {
    for (u32 i = 0; i < m_packAlphas.count(); i++) {
        u32 packIndex = m_rowIndex + i;
        if (static_cast<s32>(i) >= rowOffset && static_cast<s32>(i) < 5 + rowOffset &&
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
    if (m_rowIndex + rowOffset + 5 < m_packCount) {
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
