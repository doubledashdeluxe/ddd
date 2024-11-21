#include "SceneServerSelect.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/Client.hh>
#include <payload/online/ServerManager.hh>

extern "C" {
#include <string.h>
}

SceneServerSelect::SceneServerSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    m_mainScreen.set("GDIndexLayout.blo", 0x20000, m_archive);
    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_serverScreens[i].set("Line.blo", 0x20000, m_archive);
    }

    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_serverScreens[i].search("GDCurs")->setHasARTrans(false, true);
        m_serverScreens[i].search("GDCurs")->setHasARShift(false, true);
        m_serverScreens[i].search("GDCurs")->setHasARScale(false, true);
    }

    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_mainScreen.search("Num%02u", i)->appendChild(&m_serverScreens[i]);
    }
    m_mainScreen.search("NSaveGD")->m_isVisible = false;
    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_serverScreens[i].search("Desc41")->m_isVisible = false;
        m_serverScreens[i].search("CIcon")->m_isVisible = false;
        m_serverScreens[i].search("CCount")->m_isVisible = false;
    }

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectServerLayout.bck", m_archive);
    m_mainScreen.setAnimation(m_mainAnmTransform);
    m_arrowAnmTransform = J2DAnmLoaderDataBase::Load("SelectServerLayout.bck", m_archive);
    for (u32 i = 0; i < 2; i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAnimation(m_arrowAnmTransform);
    }
    for (u32 i = 0; i < m_serverAnmTransforms.count(); i++) {
        m_serverAnmTransforms[i] = J2DAnmLoaderDataBase::Load("GDIndexLine.bck", m_archive);
        m_serverScreens[i].setAnimation(m_serverAnmTransforms[i]);
    }
    m_descAnmTransform = J2DAnmLoaderDataBase::Load("LineDesc.bck", m_archive);
    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_serverScreens[i].search("Desc")->setAnimation(m_descAnmTransform);
    }

    m_mainAnmTransformFrame = 0;
    m_arrowAnmTransformFrame = 0;
    m_serverAnmTransformFrames.fill(0);
    m_descAnmTransformFrame = 0;
    m_arrowAlphas.fill(0);
    m_serverAlphas.fill(0);
}

SceneServerSelect::~SceneServerSelect() {}

void SceneServerSelect::init() {
    if (SequenceApp::Instance()->prevScene() != SceneType::RoomTypeSelect) {
        Client::Instance()->reset();
    }

    if (ServerManager::Instance()->lock()) {
        idle();
    } else {
        wait();
    }
}

void SceneServerSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
}

void SceneServerSelect::calc() {
    Client *client = Client::Instance();
    client->read(*this);

    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_arrowAnmTransformFrame = (m_arrowAnmTransformFrame + 1) % 35;
    for (u32 i = 0; i < 6; i++) {
        u32 serverIndex = m_rowIndex + i;
        if (serverIndex == m_serverIndex) {
            if (m_serverAnmTransformFrames[i] < 7) {
                m_serverAnmTransformFrames[i]++;
            }
        } else {
            if (m_serverAnmTransformFrames[i] > 0) {
                m_serverAnmTransformFrames[i]--;
            }
        }
    }

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_arrowAnmTransform->m_frame = m_arrowAnmTransformFrame;
    for (u32 i = 0; i < m_serverAnmTransforms.count(); i++) {
        m_serverAnmTransforms[i]->m_frame = m_serverAnmTransformFrames[i];
    }
    m_descAnmTransform->m_frame = m_descAnmTransformFrame;

    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }
    for (u32 i = 0; i < m_serverAlphas.count(); i++) {
        m_serverScreens[i].search("GDCurs")->setAlpha(m_serverAlphas[i]);
        m_serverScreens[i].search("GDCurs1")->setAlpha(m_serverAlphas[i]);
        for (u32 j = 0; j < 26; j++) {
            m_serverScreens[i].search("Name%u", j)->setAlpha(m_serverAlphas[i]);
        }
        for (u32 j = 0; j < 41; j++) {
            m_serverScreens[i].search("Desc%u", j)->setAlpha(m_serverAlphas[i]);
        }
        m_serverScreens[i].search("PIcon")->setAlpha(m_serverAlphas[i]);
        for (u32 j = 1; j <= 3; j++) {
            for (u32 k = 0; k < j; k++) {
                m_serverScreens[i].search("PCount%u%u", j, k)->setAlpha(m_serverAlphas[i]);
            }
        }
    }

    m_mainScreen.animation();
    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_serverScreens[i].animationMaterials();
    }

    client->writeStateServer();
}

bool SceneServerSelect::clientStateServer() {
    return true;
}

void SceneServerSelect::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneServerSelect::wait() {
    m_state = &SceneServerSelect::stateWait;
}

void SceneServerSelect::slideIn() {
    m_serverCount = ServerManager::Instance()->serverCount();
    if (SequenceApp::Instance()->prevScene() != SceneType::RoomTypeSelect) {
        m_serverIndex = 0;
    }
    m_rowIndex = m_serverIndex;
    m_rowIndex = Min(m_rowIndex, m_serverIndex - Min<u32>(m_serverCount, 5));

    MenuTitleLine::Instance()->drop("SelectServer.bti");
    for (u32 i = 0; i < m_serverAlphas.count(); i++) {
        u32 serverIndex = m_rowIndex + i;
        if (i < 5 && serverIndex < m_serverCount) {
            m_serverAlphas[i] = 255;
        } else {
            m_serverAlphas[i] = 0;
        }
    }
    refreshServers();
    m_state = &SceneServerSelect::stateSlideIn;
}

void SceneServerSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneServerSelect::stateSlideOut;
}

void SceneServerSelect::idle() {
    m_state = &SceneServerSelect::stateIdle;
}

void SceneServerSelect::scrollUp() {
    m_serverIndex--;
    m_rowIndex--;
    m_mainAnmTransformFrame = 46;
    m_serverAnmTransformFrames.rotateRight(1);
    m_serverAlphas.rotateRight(1);
    refreshServers();
    m_state = &SceneServerSelect::stateScrollUp;
}

void SceneServerSelect::scrollDown() {
    m_serverIndex++;
    m_mainAnmTransformFrame = 40;
    m_state = &SceneServerSelect::stateScrollDown;
}

void SceneServerSelect::nextScene() {
    m_state = &SceneServerSelect::stateNextScene;
}

void SceneServerSelect::stateWait() {
    if (ServerManager::Instance()->lock()) {
        slideIn();
    }
}

void SceneServerSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 30) {
        m_mainAnmTransformFrame++;
        if (m_mainAnmTransformFrame > 25) {
            showArrows(0);
        }
    } else {
        idle();
    }
}

void SceneServerSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        hideArrows();
    } else {
        nextScene();
    }
}

void SceneServerSelect::stateIdle() {
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.risingEdge() & PAD_BUTTON_A) {
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        u8 padCount = SequenceInfo::Instance().m_padCount;
        m_nextScene = padCount == 1 ? SceneType::NameSelect : SceneType::TandemSelect;
        slideOut();
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        if (m_serverIndex >= 1) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_serverIndex == m_rowIndex) {
                scrollUp();
            } else {
                m_serverIndex--;
            }
        }
    } else if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        if (m_serverIndex + 1 < m_serverCount) {
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CURSOL);
            if (m_serverIndex == m_rowIndex + 4) {
                scrollDown();
            } else {
                m_serverIndex++;
            }
        }
    }
}

void SceneServerSelect::stateScrollUp() {
    m_mainAnmTransformFrame--;
    showServers(0);
    showArrows(0);
    if (m_mainAnmTransformFrame == 39) {
        idle();
    }
}

void SceneServerSelect::stateScrollDown() {
    m_mainAnmTransformFrame++;
    showServers(1);
    showArrows(1);
    if (m_mainAnmTransformFrame == 47) {
        m_rowIndex++;
        m_mainAnmTransformFrame = 39;
        m_serverAnmTransformFrames.rotateLeft(1);
        m_serverAlphas.rotateLeft(1);
        refreshServers();
        idle();
    }
}

void SceneServerSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneServerSelect::refreshServers() {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    ServerManager *serverManager = ServerManager::Instance();
    for (u32 i = 0; i < 6; i++) {
        u32 serverIndex = m_rowIndex + i;
        if (serverIndex >= m_serverCount) {
            break;
        }
        const ServerManager::Server &server = serverManager->server(serverIndex);
        J2DScreen &screen = m_serverScreens[i];
        kart2DCommon->changeUnicodeTexture(server.name(), 26, screen, "Name");
        const char *desc = "Server description";
        if (serverIndex == 3) {
            desc = "This server description is too long to be fully displayed";
        }
        kart2DCommon->changeUnicodeTexture(desc, 41, screen, "Desc");
        u16 playerCounts[] = {1, 23, 456};
        u16 playerCount = playerCounts[serverIndex % 3];
        kart2DCommon->changeNumberTexture(playerCount, 3, screen, "PCount");
    }
}

void SceneServerSelect::showServers(s32 rowOffset) {
    for (u32 i = 0; i < m_serverAlphas.count(); i++) {
        u32 serverIndex = m_rowIndex + i;
        if (static_cast<s32>(i) >= rowOffset && static_cast<s32>(i) < 5 + rowOffset &&
                serverIndex < m_serverCount) {
            if (m_serverAlphas[i] < 255) {
                m_serverAlphas[i] += 51;
            }
        } else {
            if (m_serverAlphas[i] > 0) {
                m_serverAlphas[i] -= 51;
            }
        }
    }
}

void SceneServerSelect::showArrows(s32 rowOffset) {
    if (m_rowIndex + rowOffset > 0) {
        if (m_arrowAlphas[0] < 255) {
            m_arrowAlphas[0] += 51;
        }
    } else {
        if (m_arrowAlphas[0] > 0) {
            m_arrowAlphas[0] -= 51;
        }
    }
    if (m_rowIndex + rowOffset + 5 < m_serverCount) {
        if (m_arrowAlphas[1] < 255) {
            m_arrowAlphas[1] += 51;
        }
    } else {
        if (m_arrowAlphas[1] > 0) {
            m_arrowAlphas[1] -= 51;
        }
    }
}

void SceneServerSelect::hideArrows() {
    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        if (m_arrowAlphas[i] > 0) {
            m_arrowAlphas[i] -= 51;
        }
    }
}
