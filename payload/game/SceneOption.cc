#include "SceneOption.hh"

#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/System.hh"
#include "game/SystemRecord.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <jsystem/J2DPicture.hh>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

SceneOption::SceneOption(JKRArchive *archive, JKRHeap *heap)
    : Scene(archive, heap), m_titleLine(archive, heap, "option_title_line") {
    m_entryIndex = 0;
    m_rowIndex = 0;

    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *ghostDataArchive = sceneFactory->archive(SceneFactory::ArchiveType::GhostData);

    m_backgroundScreen.set("OptionBack.blo", 0x1040000, m_archive);
    m_mainScreen.set("Option.blo", 0x40000, m_archive);
    m_arrowScreen.set("GDIndexLayout.blo", 0x20000, ghostDataArchive);

    m_backgroundScreen.search("Back")->setHasARScale(false, false);
    m_backgroundScreen.search("B_mozi")->setHasARScale(false, false);

    m_arrowScreen.search("NSaveGD")->m_isVisible = false;

    m_backgroundAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("OptionBack.btk", m_archive);
    m_backgroundScreen.setAnimation(m_backgroundAnmTextureSRTKey);
    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
    m_mainScreen.search("NOpAll")->setAnimation(m_mainAnmTransform);
    m_scrollAnmTransform = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
    m_mainScreen.search("NScroll")->setAnimation(m_scrollAnmTransform);
    m_cursorAnmTransform = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
    for (u32 i = 0; i < Entry::Count; i++) {
        m_mainScreen.search("%sCs", EntryNames[i])->setAnimation(m_cursorAnmTransform);
    }
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
        m_mainScreen.search("%s_M", EntryNames[i])->setAnimation(m_entryAnmTransforms[i]);
        if (i < Option::Count) {
            m_mainScreen.search("NSb%s", EntryNames[i])->setAnimation(m_entryAnmTransforms[i]);
            if (i == Option::Volume) {
                m_mainScreen.search("NSbBGM1")->setAnimation(m_entryAnmTransforms[i]);
            }
        }
    }
    for (u32 i = 0; i < m_optionAnmTransforms.count(); i++) {
        m_optionAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
        if (i == Option::Volume) {
            for (u32 j = 0; j < 3; j++) {
                m_mainScreen.search("SbBGMw%u", j)->setAnimation(m_optionAnmTransforms[i]);
            }
        } else {
            m_mainScreen.search("Sb%s_M", EntryNames[i])->setAnimation(m_optionAnmTransforms[i]);
        }
        if (i == Option::Laps) {
            m_mainScreen.search("SbLp_M1")->setAnimation(m_optionAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_optionLeftAnmTransforms.count(); i++) {
        m_optionLeftAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
        m_mainScreen.search("NSb%sL2", EntryNames[i])->setAnimation(m_optionLeftAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_optionLeftAnmTevRegKeys.count(); i++) {
        m_optionLeftAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("Option.brk", m_archive);
        m_optionLeftAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("Sb%sL1", EntryNames[i])->setAnimation(m_optionLeftAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_optionRightAnmTransforms.count(); i++) {
        m_optionRightAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
        m_mainScreen.search("NSb%sR2", EntryNames[i])->setAnimation(m_optionRightAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_optionRightAnmTevRegKeys.count(); i++) {
        m_optionRightAnmTevRegKeys[i] = J2DAnmLoaderDataBase::Load("Option.brk", m_archive);
        m_optionRightAnmTevRegKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        m_mainScreen.search("Sb%sR1", EntryNames[i])->setAnimation(m_optionRightAnmTevRegKeys[i]);
    }
    for (u32 i = 0; i < m_optionLoopAnmTransforms.count(); i++) {
        m_optionLoopAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
        for (u32 j = 0; j < 2; j++) {
            m_mainScreen.search("NSb%s%c1", EntryNames[i], "LR"[j])
                    ->setAnimation(m_optionLoopAnmTransforms[i]);
        }
    }
    for (u32 i = 0; i < m_optionLoopAnmTextureSRTKeys.count(); i++) {
        m_optionLoopAnmTextureSRTKeys[i] = J2DAnmLoaderDataBase::Load("Option.btk", m_archive);
        m_optionLoopAnmTextureSRTKeys[i]->searchUpdateMaterialID(&m_mainScreen);
        for (u32 j = 0; j < 2; j++) {
            m_mainScreen.search("Sb%s%c2", EntryNames[i], "LR"[j])
                    ->setAnimation(m_optionLoopAnmTextureSRTKeys[i]);
        }
    }
    m_volumeAnmTransform = J2DAnmLoaderDataBase::Load("Option.bck", m_archive);
    m_mainScreen.search("BGMKnob")->setAnimation(m_volumeAnmTransform);
    m_volumeAnmTextureSRTKey = J2DAnmLoaderDataBase::Load("Option.btk", m_archive);
    m_volumeAnmTextureSRTKey->searchUpdateMaterialID(&m_mainScreen);
    m_mainScreen.search("BGMBar2")->setAnimation(m_volumeAnmTextureSRTKey);
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("OptionArrow.bck", m_archive);
    for (u32 i = 0; i < 2; i++) {
        m_arrowScreen.search("MArrow%02u", i + 1)->setAnimation(m_arrowAnmTransform);
    }

    m_backgroundAnmTextureSRTKeyFrame = 0;
    m_scrollAnmTransformFrame = 200;
    m_cursorAnmTransformFrame = 0;
    m_entryAnmTransformFrames.fill(20);
    m_optionAnmTransformFrames.fill(30);
    m_optionLeftAnmTransformFrames.fill(30);
    m_optionLeftAnmTevRegKeyFrames.fill(0);
    m_optionRightAnmTransformFrames.fill(30);
    m_optionRightAnmTevRegKeyFrames.fill(0);
    m_optionLoopAnmTransformFrames.fill(40);
    m_optionLoopAnmTextureSRTKeyFrames.fill(0);
    m_arrowAnmTransformFrame = 0;
}

SceneOption::~SceneOption() {}

void SceneOption::init() {
    m_titleLine.init(8);

    readOptions();
    m_timeBeforeRumbleEnd = 0;

    slideIn();
}

void SceneOption::draw() {
    m_graphContext->setViewport();

    m_backgroundScreen.draw(0.0f, 0.0f, m_graphContext);
    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);

    m_titleLine.draw(m_graphContext);

    m_arrowScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneOption::calc() {
    (this->*m_state)();

    if (m_timeBeforeRumbleEnd > 0) {
        m_timeBeforeRumbleEnd--;
        if (m_timeBeforeRumbleEnd == 0) {
            KartGamePad::GamePad(0)->stopMotor(true);
        }
    }

    m_backgroundAnmTextureSRTKeyFrame = (m_backgroundAnmTextureSRTKeyFrame + 1) % 540;
    m_cursorAnmTransformFrame = (m_cursorAnmTransformFrame + 1) % 90;
    for (u32 i = 0; i < Entry::Count; i++) {
        if (i == m_entryIndex) {
            if (m_entryAnmTransformFrames[i] < 25) {
                m_entryAnmTransformFrames[i]++;
            }
        } else {
            if (m_entryAnmTransformFrames[i] > 20) {
                m_entryAnmTransformFrames[i]--;
            }
        }
    }
    for (u32 i = 0; i < Option::Count; i++) {
        if (m_optionAnmTransformFrames[i] > 30) {
            m_optionAnmTransformFrames[i]++;
            if (m_optionAnmTransformFrames[i] == 38) {
                m_optionAnmTransformFrames[i] = 30;
            }
        }
        if (m_optionLeftAnmTransformFrames[i] > 30) {
            m_optionLeftAnmTransformFrames[i]++;
            if (m_optionLeftAnmTransformFrames[i] == 38) {
                m_optionLeftAnmTransformFrames[i] = 30;
            }
        }
        if (m_optionRightAnmTransformFrames[i] > 30) {
            m_optionRightAnmTransformFrames[i]++;
            if (m_optionRightAnmTransformFrames[i] == 38) {
                m_optionRightAnmTransformFrames[i] = 30;
            }
        }
        if (i == m_entryIndex) {
            if (m_optionLeftAnmTevRegKeyFrames[i] > 1 && m_optionLeftAnmTevRegKeyFrames[i] < 19) {
                m_optionLeftAnmTevRegKeyFrames[i]++;
            } else {
                m_optionLeftAnmTevRegKeyFrames[i] = 1;
            }
            if (m_optionRightAnmTevRegKeyFrames[i] > 1 && m_optionRightAnmTevRegKeyFrames[i] < 19) {
                m_optionRightAnmTevRegKeyFrames[i]++;
            } else {
                m_optionRightAnmTevRegKeyFrames[i] = 1;
            }
            m_optionLoopAnmTransformFrames[i]++;
            if (m_optionLoopAnmTransformFrames[i] > 70) {
                m_optionLoopAnmTransformFrames[i] = 40;
            }
            m_optionLoopAnmTextureSRTKeyFrames[i]++;
            if (m_optionLoopAnmTextureSRTKeyFrames[i] > 119) {
                m_optionLoopAnmTextureSRTKeyFrames[i] = 0;
            }
        } else {
            m_optionLeftAnmTevRegKeyFrames[i] = 0;
            m_optionRightAnmTevRegKeyFrames[i] = 0;
            m_optionLoopAnmTransformFrames[i] = 40;
            m_optionLoopAnmTextureSRTKeyFrames[i] = 0;
        }
    }
    m_volumeAnmTransformFrame = 100.0f + m_unsavedValues[Option::Volume] * 99.0f / 20.0f;
    m_volumeAnmTextureSRTKeyFrame = m_unsavedValues[Option::Volume] * 99.0f / 20.0f;
    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 35;

    m_backgroundAnmTextureSRTKey->m_frame = m_backgroundAnmTextureSRTKeyFrame;
    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_scrollAnmTransform->m_frame = m_scrollAnmTransformFrame;
    m_cursorAnmTransform->m_frame = m_cursorAnmTransformFrame;
    for (u32 i = 0; i < m_entryAnmTransforms.count(); i++) {
        m_entryAnmTransforms[i]->m_frame = m_entryAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_optionAnmTransforms.count(); i++) {
        m_optionAnmTransforms[i]->m_frame = m_optionAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_optionLeftAnmTransforms.count(); i++) {
        m_optionLeftAnmTransforms[i]->m_frame = m_optionLeftAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_optionLeftAnmTevRegKeys.count(); i++) {
        m_optionLeftAnmTevRegKeys[i]->m_frame = m_optionLeftAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_optionRightAnmTransforms.count(); i++) {
        m_optionRightAnmTransforms[i]->m_frame = m_optionRightAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_optionRightAnmTevRegKeys.count(); i++) {
        m_optionRightAnmTevRegKeys[i]->m_frame = m_optionRightAnmTevRegKeyFrames[i];
    }
    for (u32 i = 0; i < m_optionLoopAnmTransforms.count(); i++) {
        m_optionLoopAnmTransforms[i]->m_frame = m_optionLoopAnmTransformFrames[i];
    }
    for (u32 i = 0; i < m_optionLoopAnmTextureSRTKeys.count(); i++) {
        m_optionLoopAnmTextureSRTKeys[i]->m_frame = m_optionLoopAnmTextureSRTKeyFrames[i];
    }
    m_volumeAnmTransform->m_frame = m_volumeAnmTransformFrame;
    m_volumeAnmTextureSRTKey->m_frame = m_volumeAnmTextureSRTKeyFrame;
    m_arrowAnmTransform->m_frame = m_arrowAnmTransformFrame;

    for (u32 i = 0; i < m_optionAlphas.count(); i++) {
        m_mainScreen.search("%s_M", EntryNames[i])->setAlpha(m_optionAlphas[i]);
        m_mainScreen.search("%sCs", EntryNames[i])->setAlpha(m_optionAlphas[i]);
        if (i == Option::Volume) {
            for (u32 j = 0; j < 3; j++) {
                m_mainScreen.search("SbBGMw%u", j)->setAlpha(m_optionAlphas[i]);
            }
            m_mainScreen.search("BGMBar1")->setAlpha(m_optionAlphas[i]);
            m_mainScreen.search("BGMBar2")->setAlpha(m_optionAlphas[i]);
            m_mainScreen.search("BGMKnob")->setAlpha(m_optionAlphas[i]);
        } else {
            m_mainScreen.search("Sb%s_M", EntryNames[i])->setAlpha(m_optionAlphas[i]);
        }
        if (i == Option::Laps) {
            m_mainScreen.search("SbLp_M1")->setAlpha(m_optionAlphas[i]);
        }
        for (u32 j = 0; j < 2; j++) {
            for (u32 k = 0; k < 2; k++) {
                m_mainScreen.search("Sb%s%c%u", EntryNames[i], "LR"[j], k + 1)
                        ->setAlpha(m_optionAlphas[i]);
            }
        }
    }
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_arrowScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }

    for (u32 i = 0; i < Entry::Count; i++) {
        m_mainScreen.search("%sCs", EntryNames[i])->m_isVisible = i == m_entryIndex;
    }
    m_mainScreen.search("SbBGMw0")->m_isVisible = m_unsavedValues[Option::Volume] == 10;
    m_mainScreen.search("SbBGMw1")->m_isVisible = m_unsavedValues[Option::Volume] != 10;
    m_mainScreen.search("SbBGMw2")->m_isVisible = m_unsavedValues[Option::Volume] != 10;
    m_mainScreen.search("SbLp_M")->m_isVisible = m_unsavedValues[Option::Laps] != 0;
    m_mainScreen.search("SbLp_M1")->m_isVisible = m_unsavedValues[Option::Laps] == 0;

    for (u32 i = 0; i < Entry::Count; i++) {
        GXColor black, white;
        J2DPicture *picture =
                m_mainScreen.search("CSel%u", i == m_entryIndex ? 1 : 2)->downcast<J2DPicture>();
        picture->getBlackWhite(&black, &white);
        picture = m_mainScreen.search("%s_M", EntryNames[i])->downcast<J2DPicture>();
        picture->setBlackWhite(black, white);
        if (i == Option::Volume) {
            for (u32 j = 0; j < 3; j++) {
                picture = m_mainScreen.search("SbBGMw%u", j)->downcast<J2DPicture>();
                picture->setBlackWhite(black, white);
            }
        } else if (i < Option::Count) {
            picture = m_mainScreen.search("Sb%s_M", EntryNames[i])->downcast<J2DPicture>();
            picture->setBlackWhite(black, white);
        }
        if (i == Option::Laps) {
            picture = m_mainScreen.search("SbLp_M1")->downcast<J2DPicture>();
            picture->setBlackWhite(black, white);
        }
    }

    refreshOption("SbBGMw1", static_cast<u8>(abs(m_unsavedValues[Option::Volume] - 10)));
    refreshOption("SbBGMw2", "-+"[m_unsavedValues[Option::Volume] > 10]);
    const char *soundValueNames[] = {"Mono", "Stereo", "DolbySurround"};
    refreshOption(Option::Sound, soundValueNames);
    const char *rumbleValueNames[] = {"Off", "On"};
    refreshOption(Option::Rumble, rumbleValueNames);
    const char *ghostsValueNames[] = {"Off", "On"};
    refreshOption(Option::Ghosts, ghostsValueNames);
    const char *itemsValueNames[] = {"Standard", "Smash", "Super", "Simple"};
    refreshOption(Option::Items, itemsValueNames);
    refreshOption(Option::Laps, static_cast<s8>(0));
    refreshOption(Option::P1RearView, "XYLR");
    refreshOption(Option::P2RearView, "XYLR");
    refreshOption(Option::P3RearView, "XYLR");
    refreshOption(Option::P4RearView, "XYLR");
    const char *aspectRatioValueNames[] = {"Auto", "43", "1914", "32", "1610", "169", "3821",
            "219"};
    refreshOption(Option::AspectRatio, aspectRatioValueNames);

    m_backgroundScreen.animation();
    m_mainScreen.animation();
    m_arrowScreen.animation();

    m_titleLine.calcMatAnm();
    m_titleLine.calcAnm(m_titleLineDirection);
}

void SceneOption::slideIn() {
    m_mainAnmTransformFrame = 0;
    m_titleLineDirection = 1;
    System::GetDisplay()->startFadeIn(15);
    GameAudio::Main::Instance()->startSequenceBgm(SoundID::JA_BGM_OPTION);
    for (u32 i = 0; i < m_optionAlphas.count(); i++) {
        m_optionAlphas[i] = i - m_rowIndex < 6 ? 255 : 0;
    }
    m_arrowAlphas.fill(0);
    m_state = &SceneOption::stateSlideIn;
}

void SceneOption::slideOut() {
    m_mainAnmTransformFrame = 80;
    m_titleLineDirection = -1;
    GameAudio::Main::Instance()->fadeOutAll(15);
    System::GetDisplay()->startFadeOut(15);
    m_state = &SceneOption::stateSlideOut;
}

void SceneOption::idle() {
    m_titleLineDirection = 0;
    m_state = &SceneOption::stateIdle;
}

void SceneOption::scrollUp() {
    m_entryIndex--;
    m_rowIndex--;
    m_scrollAnmTransformFrame--;
    m_state = &SceneOption::stateScrollUp;
}

void SceneOption::scrollDown() {
    m_entryIndex++;
    m_rowIndex++;
    m_scrollAnmTransformFrame++;
    m_state = &SceneOption::stateScrollDown;
}

void SceneOption::wait() {
    CardAgent::Ask(CardAgent::Command::WriteSystemFile, 0);
    m_state = &SceneOption::stateWait;
}

void SceneOption::nextScene() {
    m_state = &SceneOption::stateNextScene;
}

void SceneOption::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        if (m_mainAnmTransformFrame > 10) {
            showArrows();
        }
    } else {
        idle();
    }
}

void SceneOption::stateSlideOut() {
    if (m_mainAnmTransformFrame < 95) {
        m_mainAnmTransformFrame++;
        hideArrows();
    } else {
        nextScene();
    }
}

void SceneOption::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
        if (m_entryIndex == Option::Count) {
            bool hasChanged = false;
            for (u32 i = 0; i < Option::Count; i++) {
                if (m_unsavedValues[i] != m_savedValues[i]) {
                    hasChanged = true;
                    break;
                }
            }
            if (hasChanged) {
                writeOptions();
                wait();
            } else {
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
                slideOut();
            }
        }
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        GameAudio::Main::Instance()->setOutputMode(m_savedValues[Option::Sound]);
        GameAudio::Main::Instance()->setMasterVolume(m_savedValues[Option::Volume] - 10);
        System::SetAspectRatio(m_savedValues[Option::AspectRatio]);
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_entryIndex >= 1) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_entryIndex == m_rowIndex) {
                scrollUp();
            } else {
                m_entryIndex--;
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_entryIndex + 1 < Entry::Count) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_entryIndex == m_rowIndex + 5 && m_entryIndex + 1 != Option::Count) {
                scrollDown();
            } else {
                m_entryIndex++;
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_LEFT) {
        if (m_entryIndex < Option::Count) {
            if (m_unsavedValues[m_entryIndex] > 0) {
                m_unsavedValues[m_entryIndex]--;
                onOptionChange();
            } else if (m_entryIndex != Option::Volume) {
                m_unsavedValues[m_entryIndex] = ValueCounts[m_entryIndex] - 1;
                onOptionChange();
            }
            m_optionAnmTransformFrames[m_entryIndex] = 31;
            m_optionLeftAnmTransformFrames[m_entryIndex] = 31;
            m_optionLeftAnmTevRegKeyFrames[m_entryIndex] = 2;
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_RIGHT) {
        if (m_entryIndex < Option::Count) {
            if (m_unsavedValues[m_entryIndex] < ValueCounts[m_entryIndex] - 1) {
                m_unsavedValues[m_entryIndex]++;
                onOptionChange();
            } else if (m_entryIndex != Option::Volume) {
                m_unsavedValues[m_entryIndex] = 0;
                onOptionChange();
            }
            m_optionAnmTransformFrames[m_entryIndex] = 31;
            m_optionRightAnmTransformFrames[m_entryIndex] = 31;
            m_optionRightAnmTevRegKeyFrames[m_entryIndex] = 2;
        }
    }
}

void SceneOption::stateScrollUp() {
    m_scrollAnmTransformFrame--;
    showOptions();
    showArrows();
    if (m_scrollAnmTransformFrame % 8 == 0) {
        idle();
    }
}

void SceneOption::stateScrollDown() {
    m_scrollAnmTransformFrame++;
    showOptions();
    showArrows();
    if (m_scrollAnmTransformFrame % 8 == 0) {
        idle();
    }
}

void SceneOption::stateWait() {
    if (!CardAgent::IsReady()) {
        return;
    }

    CardAgent::Ack(SoundID::JA_SE_TR_DECIDE);
    slideOut();
}

void SceneOption::stateNextScene() {
    u32 nextScene = SceneType::Title;
    if (!SequenceApp::Instance()->ready(nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(nextScene);
}

void SceneOption::readOptions() {
    SystemRecord &systemRecord = SystemRecord::Instance();
    if (systemRecord.m_soundMode >= 1 && systemRecord.m_soundMode <= 2) {
        m_savedValues[Option::Sound] = systemRecord.m_soundMode / 2 * 2;
    } else {
        m_savedValues[Option::Sound] = 1;
    }
    m_savedValues[Option::Volume] = systemRecord.m_volume + 10;
    m_savedValues[Option::Rumble] = !systemRecord.m_noRumble;
    m_savedValues[Option::Ghosts] = !systemRecord.m_noTAGhosts;
    if (systemRecord.m_itemSlotType >= 1 && systemRecord.m_itemSlotType <= 3) {
        m_savedValues[Option::Items] = 1 + (systemRecord.m_itemSlotType + 1) % 3;
    } else {
        m_savedValues[Option::Items] = 0;
    }
    m_savedValues[Option::Laps] = systemRecord.m_vsLapNum;
    for (u32 i = 0; i < 4; i++) {
        m_savedValues[Option::P1RearView + i] = (systemRecord.m_rearViewButtons >> (2 * i)) % 4;
    }
    m_savedValues[Option::AspectRatio] = systemRecord.m_aspectRatio;

    m_unsavedValues = m_savedValues;
}

void SceneOption::writeOptions() {
    SystemRecord &systemRecord = SystemRecord::Instance();
    if (m_unsavedValues[Option::Sound] >= 1 && m_unsavedValues[Option::Sound] <= 2) {
        systemRecord.m_soundMode = m_unsavedValues[Option::Sound] / 2 * 2;
    } else {
        systemRecord.m_soundMode = 1;
    }
    systemRecord.m_volume = m_unsavedValues[Option::Volume] - 10;
    systemRecord.m_noRumble = !m_unsavedValues[Option::Rumble];
    systemRecord.m_noTAGhosts = !m_unsavedValues[Option::Ghosts];
    systemRecord.m_vsLapNum = m_unsavedValues[Option::Laps];
    if (m_unsavedValues[Option::Items] >= 1 && m_unsavedValues[Option::Items] <= 3) {
        systemRecord.m_itemSlotType = 1 + (m_unsavedValues[Option::Items] - 1) % 3;
    } else {
        systemRecord.m_itemSlotType = 0;
    }
    systemRecord.m_rearViewButtons = 0;
    for (u32 i = 0; i < 4; i++) {
        systemRecord.m_rearViewButtons |= m_unsavedValues[Option::P1RearView + i] << (2 * i);
    }
    systemRecord.m_aspectRatio = m_unsavedValues[Option::AspectRatio];

    m_savedValues = m_unsavedValues;
}

void SceneOption::onOptionChange() {
    if (m_entryIndex == Option::Sound) {
        GameAudio::Main::Instance()->setOutputMode(m_unsavedValues[Option::Sound]);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_SOUNDTEST);
    } else if (m_entryIndex == Option::Volume) {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
        GameAudio::Main::Instance()->setMasterVolume(m_unsavedValues[Option::Volume] - 10);
    } else {
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE2);
    }
    if (m_entryIndex == Option::Rumble && m_unsavedValues[Option::Rumble]) {
        KartGamePad::GamePad(0)->startMotor();
        m_timeBeforeRumbleEnd = 10;
    }
    if (m_entryIndex == Option::AspectRatio) {
        System::SetAspectRatio(m_unsavedValues[Option::AspectRatio]);
    }
}

void SceneOption::refreshOption(u32 index, const char **valueNames) {
    Array<char, 9> tag;
    snprintf(tag.values(), tag.count(), "Sb%s_M", EntryNames[index]);
    Array<char, 32> name;
    snprintf(name.values(), name.count(), "Mozi_%s.bti", valueNames[m_unsavedValues[index]]);
    refreshOption(tag.values(), name.values());
}

void SceneOption::refreshOption(u32 index, const char *valueNames) {
    Array<char, 9> tag;
    snprintf(tag.values(), tag.count(), "Sb%s_M", EntryNames[index]);
    char name = valueNames[m_unsavedValues[index]];
    refreshOption(tag.values(), name);
}

void SceneOption::refreshOption(u32 index, s8 nameOffset) {
    Array<char, 9> tag;
    snprintf(tag.values(), tag.count(), "Sb%s_M", EntryNames[index]);
    u8 name = m_unsavedValues[index] + nameOffset;
    refreshOption(tag.values(), name);
}

void SceneOption::refreshOption(const char *tag, const char *name) {
    J2DPicture *picture = m_mainScreen.search("%s", tag)->downcast<J2DPicture>();
    picture->changeTexture(name, 0);
}

void SceneOption::refreshOption(const char *tag, char name) {
    J2DPicture *picture = m_mainScreen.search("%s", tag)->downcast<J2DPicture>();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    picture->changeTexture(kart2DCommon->getAsciiTexture(name), 0);
}

void SceneOption::refreshOption(const char *tag, u8 name) {
    J2DPicture *picture = m_mainScreen.search("%s", tag)->downcast<J2DPicture>();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    picture->changeTexture(kart2DCommon->getNumberTexture(name), 0);
}

void SceneOption::showOptions() {
    for (u32 i = 0; i < m_optionAlphas.count(); i++) {
        if (i - m_rowIndex < 6) {
            if (m_optionAlphas[i] < 255) {
                m_optionAlphas[i] += 51;
            }
        } else {
            if (m_optionAlphas[i] > 0) {
                m_optionAlphas[i] -= 51;
            }
        }
    }
}

void SceneOption::showArrows() {
    if (m_rowIndex > 0) {
        if (m_arrowAlphas[0] < 255) {
            m_arrowAlphas[0] += 51;
        }
    } else {
        if (m_arrowAlphas[0] > 0) {
            m_arrowAlphas[0] -= 51;
        }
    }
    if (m_rowIndex + 6 < Option::Count) {
        if (m_arrowAlphas[1] < 255) {
            m_arrowAlphas[1] += 51;
        }
    } else {
        if (m_arrowAlphas[1] > 0) {
            m_arrowAlphas[1] -= 51;
        }
    }
}

void SceneOption::hideArrows() {
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        if (m_arrowAlphas[i] > 0) {
            m_arrowAlphas[i] -= 51;
        }
    }
}

const u32 SceneOption::ValueCounts[Option::Count] = {3, 21, 2, 2, 4, 10, 4, 4, 4, 4, 8};
const char *const SceneOption::EntryNames[Entry::Count] = {"Sd", "BGM", "Rm", "Gh", "Is", "Lp",
        "Re1", "Re2", "Re3", "Re4", "As", "ReTtl"};
