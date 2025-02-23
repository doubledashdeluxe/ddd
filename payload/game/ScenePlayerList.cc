#include "ScenePlayerList.hh"

#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/OnlineTimer.hh"
#include "game/Race2D.hh"
#include "game/RoomType.hh"
#include "game/SequenceApp.hh"

extern "C" {
#include <dolphin/OSTime.h>
}
#include <jsystem/J2DAnmLoaderDataBase.hh>

extern "C" {
#include <stdio.h>
}

ScenePlayerList::ScenePlayerList(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_mainScreen.set("PlayerList.blo", 0x0, m_archive);
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        m_playerScreens[i].set("PlayerListPlayer.blo", 0x0, m_archive);
    }

    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        for (u32 j = 1; j <= 3; j++) {
            m_playerScreens[i].search("RCurs%02u", j)->setHasARTrans(false, true);
            m_playerScreens[i].search("RCurs%02u", j)->setHasARShift(false, true);
            m_playerScreens[i].search("RCurs%02u", j)->setHasARScale(false, true);
        }
    }

    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        m_mainScreen.search("Player%u", i)->appendChild(&m_playerScreens[i]);
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("PlayerList.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);

    OnlineTimer::Create(m_archive);
}

ScenePlayerList::~ScenePlayerList() {}

void ScenePlayerList::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::CharacterSelect &&
            OnlineTimer::Instance()->hasExpired()) {
        m_nextScene = SceneType::CharacterSelect;
        nextScene();
    } else {
        slideIn();
    }
}

void ScenePlayerList::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);

    OnlineTimer::Instance()->draw(m_graphContext);
}

void ScenePlayerList::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;

    m_mainScreen.animation();
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        m_playerScreens[i].animationMaterials();
    }

    OnlineTimer::Instance()->calc();
}

void ScenePlayerList::slideIn() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < m_playerScreens.count(); i++) {
        J2DScreen &screen = m_playerScreens[i];
        GXColor color = Race2D::GetPlayerNumberColor(i);
        for (u32 j = 1; j <= 3; j++) {
            J2DPicture *picture = screen.search("RCurs%02u", j)->downcast<J2DPicture>();
            picture->setWhite(color);
        }
        kart2DCommon->changeUnicodeTexture("ABC", 3, screen, "PName0");
        if (i % 2) {
            kart2DCommon->changeUnicodeTexture("   ", 3, screen, "PName1");
        } else {
            kart2DCommon->changeUnicodeTexture("DEF", 3, screen, "PName1");
        }
        for (u32 j = 0; j < 2; j++) {
            for (u32 k = 0; k < 2; k++) {
                J2DPicture *picture = screen.search("P%c%u", "BN"[k], j)->downcast<J2DPicture>();
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
        Array<J2DPicture *, 4> pictures;
        for (u32 j = 0; j < pictures.count(); j++) {
            pictures[j] = screen.search("MMR%u", j)->downcast<J2DPicture>();
        }
        s32 mmr = OSGetTime() % 10000;
        kart2DCommon->changeNumberTexture(mmr, pictures.values(), pictures.count(), false, false);
    }

    MenuTitleLine::Instance()->drop("PlayerList.bti");
    m_mainAnmTransformFrame = 0;
    m_state = &ScenePlayerList::stateSlideIn;
}

void ScenePlayerList::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &ScenePlayerList::stateSlideOut;
}

void ScenePlayerList::idle() {
    m_state = &ScenePlayerList::stateIdle;
}

void ScenePlayerList::nextScene() {
    m_state = &ScenePlayerList::stateNextScene;
}

void ScenePlayerList::stateSlideIn() {
    if (m_mainAnmTransformFrame < 20) {
        m_mainAnmTransformFrame++;
        if (SequenceApp::Instance()->prevScene() != SceneType::CharacterSelect) {
            OnlineTimer *onlineTimer = OnlineTimer::Instance();
            if (m_mainAnmTransformFrame == 1) {
                onlineTimer->init(30 * 60);
            }
            if (m_mainAnmTransformFrame <= 15) {
                onlineTimer->setAlpha(m_mainAnmTransformFrame * 17);
            }
        }
    } else {
        idle();
    }
}

void ScenePlayerList::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_nextScene != SceneType::CharacterSelect) {
            if (m_mainAnmTransformFrame <= 15) {
                OnlineTimer::Instance()->setAlpha(m_mainAnmTransformFrame * 17);
            }
        }
    } else {
        nextScene();
    }
}

void ScenePlayerList::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A || OnlineTimer::Instance()->hasExpired()) {
        m_nextScene = SceneType::CharacterSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        switch (OnlineInfo::Instance().m_roomType) {
        case RoomType::Worldwide:
            m_nextScene = SceneType::FormatSelect;
            break;
        case RoomType::Duel:
            m_nextScene = SceneType::PackSelect;
            break;
        default:
            m_nextScene = SceneType::PersonalRoom;
            break;
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        slideOut();
    }
}

void ScenePlayerList::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}
