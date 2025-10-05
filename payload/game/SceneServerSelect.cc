#include "SceneServerSelect.hh"

#include "game/ErrorViewApp.hh"
#include "game/GameAudioMain.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/Client.hh>
#include <payload/online/CubeServerManager.hh>

extern "C" {
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
}

SceneServerSelect::SceneServerSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);

    m_mainScreen.set("GDIndexLayout.blo", 0x20000, m_archive);
    m_colorScreen.set("ServerColors.blo", 0x20000, lanEntryArchive);
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
        m_serverAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_serverScreens[i].setAnimation(m_serverAnmTransforms[i]);
    }
    for (u32 i = 0; i < m_descAnmTransforms.count(); i++) {
        m_descAnmTransforms[i] = J2DAnmLoaderDataBase::Load("Line.bck", m_archive);
        m_serverScreens[i].search("Desc")->setAnimation(m_descAnmTransforms[i]);
    }

    m_arrowAnmTransformFrame = 0;
    m_serverAnmTransformFrames.fill(0);
    m_descAnmTransformFrames.fill(0);
    m_arrowAlphas.fill(0);
    m_serverAlphas.fill(0);
    m_descAlphas.fill(0);
}

SceneServerSelect::~SceneServerSelect() {}

void SceneServerSelect::init() {
    if (SequenceApp::Instance()->prevScene() != SceneType::RoomTypeSelect) {
        Client::Instance()->reset();
    }

    m_serverCount = 0;
    for (u32 i = 0; i < m_descs.count(); i++) {
        // Crashes MWCC:
        // m_descs[i][0] = '\0';
        snprintf(m_descs[i].values(), m_descs[i].count(), "");
    }
    m_descOffsets.fill(0);
    m_descColorPictures.fill(nullptr);
    for (u32 i = 0; i < m_playerCounts.count(); i++) {
        // Crashes MWCC:
        // m_playerCounts[i][0] = '\0';
        snprintf(m_playerCounts[i].values(), m_playerCounts[i].count(), "");
    }

    if (CubeServerManager::Instance()->lock()) {
        slideIn();
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

    u32 increment = 5;
    const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
    if (button.level() & (PAD_TRIGGER_L | PAD_TRIGGER_R)) {
        increment += 20;
    }
    for (u32 i = 0; i < m_descOffsets.count(); i++) {
        m_descOffsets[i] += increment;
    }
    refreshServers();

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
    for (u32 i = 0; i < m_descAnmTransforms.count(); i++) {
        m_descAnmTransforms[i]->m_frame = m_descAnmTransformFrames[i];
    }

    for (u32 i = 0; i < m_arrowAlphas.count(); i++) {
        m_mainScreen.search("MArrow%02u", i + 1)->setAlpha(m_arrowAlphas[i]);
    }
    for (u32 i = 0; i < m_serverAlphas.count(); i++) {
        m_serverScreens[i].search("GDCurs")->setAlpha(m_serverAlphas[i]);
        m_serverScreens[i].search("GDCurs1")->setAlpha(m_serverAlphas[i]);
        for (u32 j = 0; j < 26; j++) {
            m_serverScreens[i].search("Name%u", j)->setAlpha(m_serverAlphas[i]);
        }
        for (u32 j = 0; j < 42; j++) {
            u8 alpha = m_serverAlphas[i];
            if (j == 0) {
                alpha = (alpha * (255 - m_descAlphas[i])) >> 8;
            } else if (j == 41) {
                alpha = (alpha * m_descAlphas[i]) >> 8;
            }
            m_serverScreens[i].search("Desc%u", j)->setAlpha(alpha);
        }
        m_serverScreens[i].search("PIcon")->setAlpha(m_serverAlphas[i]);
        for (u32 j = 0; j < 3; j++) {
            m_serverScreens[i].search("PCount%u", j)->setAlpha(m_serverAlphas[i]);
        }
    }

    m_mainScreen.animation();
    for (u32 i = 0; i < m_serverScreens.count(); i++) {
        m_serverScreens[i].animationMaterials();
    }

    client->writeStateServer();
}

SceneServerSelect::DescText::DescText(SceneServerSelect &scene, u32 descIndex)
    : m_scene(scene), m_descIndex(descIndex) {}

SceneServerSelect::DescText::~DescText() {}

const char *SceneServerSelect::DescText::getPart(u32 /* partIndex */) {
    u32 serverIndex = m_scene.m_rowIndex + m_descIndex;
    return m_scene.m_descs[serverIndex].values();
}

void SceneServerSelect::DescText::setAnmTransformFrame(u8 anmTransformFrame) {
    m_scene.m_descAnmTransformFrames[m_descIndex] = anmTransformFrame;
}

void SceneServerSelect::DescText::setAlpha(u8 alpha) {
    m_scene.m_descAlphas[m_descIndex] = alpha;
}

bool SceneServerSelect::clientStateIdle() {
    return true;
}

bool SceneServerSelect::clientStateServer(const ClientStateServerInfo &info) {
    bool networkIsRunning = info.networkIsRunning;
    const char *networkName = info.networkName;
    u32 networkAddress = info.networkAddress;
    CubeServerManager *serverManager = CubeServerManager::Instance();
    for (u32 i = 0; i < info.servers.count(); i++) {
        const ServerManager::Server &server = serverManager->server(i);
        const Array<char, 32> &name = server.address();
        u16 port = server.port();
        const ClientStateServerInfo::Server &serverInfo = info.servers[i];
        const Optional<Address> &address = serverInfo.address;
        const Optional<u32> &protocolVersion = serverInfo.protocolVersion;
        const Optional<Version> &version = serverInfo.version;
        const Optional<Array<char, 100>> &motd = serverInfo.motd;
        const Optional<u16> &uncappedPlayerCount = serverInfo.playerCount;
        bool versionIsCompatible = serverInfo.versionIsCompatible;
        Array<char, 100> prevDesc = m_descs[i];
        Array<char, 100> &desc = m_descs[i];
        if (motd) {
            desc = *motd;
        } else if (protocolVersion && version && !versionIsCompatible) {
            snprintf(desc.values(), desc.count(), "v%u.%u.%u (%" PRIu32 ")", version->major,
                    version->minor, version->patch, *protocolVersion);
        } else if (protocolVersion && version) {
            snprintf(desc.values(), desc.count(), "%s", String(7));
        } else if (networkAddress && address) {
            snprintf(desc.values(), desc.count(), "%s%s:%u%s", String(5), name.values(), port,
                    String(6));
        } else if (networkAddress) {
            snprintf(desc.values(), desc.count(), "%s%s%s", String(3), server.address().values(),
                    String(4));
        } else if (networkIsRunning) {
            snprintf(desc.values(), desc.count(), "%s", String(2));
        } else {
            snprintf(desc.values(), desc.count(), "%s%s%s", String(0), networkName, String(1));
        }
        if (strcmp(desc.values(), prevDesc.values())) {
            m_descOffsets[i] = 0;
        }
        J2DPicture *&descColorPicture = m_descColorPictures[i];
        if (motd) {
            descColorPicture = m_colorScreen.search("Ok")->downcast<J2DPicture>();
        } else if (protocolVersion && version && !versionIsCompatible) {
            descColorPicture = m_colorScreen.search("Error")->downcast<J2DPicture>();
        } else {
            descColorPicture = m_colorScreen.search("Wait")->downcast<J2DPicture>();
        }
        Array<char, 4> &playerCount = m_playerCounts[i];
        if (uncappedPlayerCount) {
            u16 cappedPlayerCount = Min<u16>(*uncappedPlayerCount, 999);
            snprintf(playerCount.values(), playerCount.count(), "%u", cappedPlayerCount);
        } else if (protocolVersion && !versionIsCompatible) {
            snprintf(playerCount.values(), playerCount.count(), "X");
        } else {
            snprintf(playerCount.values(), playerCount.count(), "...");
        }
    }

    return true;
}

void SceneServerSelect::clientStateError() {
    ErrorViewApp::Call(6);
}

void SceneServerSelect::wait() {
    m_state = &SceneServerSelect::stateWait;
}

void SceneServerSelect::slideIn() {
    m_serverCount = CubeServerManager::Instance()->serverCount();
    if (SequenceApp::Instance()->prevScene() != SceneType::RoomTypeSelect) {
        m_serverIndex = 0;
    }
    m_rowIndex = m_serverIndex;
    m_rowIndex = Min(m_rowIndex, m_serverIndex - Min<u32>(m_serverCount, 5));

    MenuTitleLine::Instance()->drop("SelectServer.bti");
    m_mainAnmTransformFrame = 0;
    for (u32 i = 0; i < m_serverAlphas.count(); i++) {
        u32 serverIndex = m_rowIndex + i;
        if (i < 5 && serverIndex < m_serverCount) {
            m_serverAlphas[i] = 255;
        } else {
            m_serverAlphas[i] = 0;
        }
    }
    m_state = &SceneServerSelect::stateSlideIn;
}

void SceneServerSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_mainAnmTransformFrame = 30;
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
    if (CubeServerManager::Instance()->lock()) {
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
        m_nextScene = SceneType::RoomTypeSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE_LITTLE);
        slideOut();
    } else if (button.risingEdge() & PAD_BUTTON_B) {
        u8 padCount = SequenceInfo::Instance().m_padCount;
        m_nextScene = padCount == 1 ? SceneType::NameSelect : SceneType::TandemSelect;
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
        OnlineInfo::Instance().m_hasIDs = false;
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
    CubeServerManager *serverManager = CubeServerManager::Instance();
    for (u32 i = 0; i < 6; i++) {
        u32 serverIndex = m_rowIndex + i;
        if (serverIndex >= m_serverCount) {
            break;
        }
        const ServerManager::Server &server = serverManager->server(serverIndex);
        J2DScreen &screen = m_serverScreens[i];
        kart2DCommon->changeUnicodeTexture(server.name().values(), 26, screen, "Name");
        DescText descText(*this, i);
        u64 descOffset = Max<u64>(m_descOffsets[serverIndex], 300) - 300;
        descText.refresh(descOffset, 1, 42, screen, "Desc");
        J2DPicture *descColorPicture = m_descColorPictures[i];
        if (descColorPicture) {
            for (u32 j = 0; j < 42; j++) {
                J2DPicture *descPicture = screen.search("Desc%u", j)->downcast<J2DPicture>();
                descPicture->m_cornerColors = descColorPicture->m_cornerColors;
            }
        }
        kart2DCommon->changeUnicodeTexture(m_playerCounts[i].values(), 3, screen, "PCount", true);
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

const char *SceneServerSelect::String(u32 index) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);
    Array<char, 32> path;
    snprintf(path.values(), path.count(), "/strings/%" PRIu32 ".txt", index);
    char *string = static_cast<char *>(lanEntryArchive->getResource(path.values()));
    u32 size = lanEntryArchive->getResSize(string);
    string[size - 1] = '\0';
    return string;
}
