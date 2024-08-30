#include "SceneServerSelect.hh"

#include "game/ErrorViewApp.hh"
#include "game/SequenceApp.hh"

#include <payload/online/Client.hh>
#include <payload/online/ServerManager.hh>

SceneServerSelect::SceneServerSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {}

SceneServerSelect::~SceneServerSelect() {}

void SceneServerSelect::init() {
    s32 prevScene = SequenceApp::Instance()->prevScene();
    if (prevScene == SceneType::NameSelect || prevScene == SceneType::TandemSelect) {
        Client::Instance()->reset();
    }

    if (ServerManager::Instance()->lock()) {
        idle();
    } else {
        wait();
    }
}

void SceneServerSelect::draw() {}

void SceneServerSelect::calc() {
    Client *client = Client::Instance();
    client->read(*this);

    (this->*m_state)();

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

void SceneServerSelect::idle() {
    m_state = &SceneServerSelect::stateIdle;
}

void SceneServerSelect::stateWait() {
    if (ServerManager::Instance()->lock()) {
        idle();
    }
}

void SceneServerSelect::stateIdle() {}
