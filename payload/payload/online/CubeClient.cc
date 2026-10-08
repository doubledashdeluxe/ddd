#include "CubeClient.hh"

#include "payload/crypto/CubeRandom.hh"
#include "payload/network/CubeDNS.hh"
#include "payload/network/CubeNetwork.hh"
#include "payload/online/ClientK.hh"
#include "payload/online/CubeReplay.hh"
#include "payload/online/CubeServerManager.hh"

#include <jsystem/JKRExpHeap.hh>
#include <portable/online/ClientStateError.hh>
#include <portable/online/ClientStateIdle.hh>

void CubeClient::reset() {
    m_state.reset();
    m_platform.replay.reset();
    updateState(*(new (m_platform.allocator) ClientStateIdle(m_platform)));
}

void CubeClient::setReplay(const ReplayManager::Replay &replay, u32 clientIndex) {
    reset();
    m_platform.replay.reset(new (m_platform.allocator) CubeReplay(replay, clientIndex));
}

void CubeClient::read(ClientReadHandler &handler) {
    do {
        if (!m_state->ok()) {
            updateState(*(new (m_platform.allocator) ClientStateError(m_platform)));
        }
    } while (updateState(m_state->read(handler)));
}

void CubeClient::Init(JKRHeap *heap, SOConfig &config) {
    s_instance = new (heap, 0x4) CubeClient(config, heap);
}

CubeClient *CubeClient::Instance() {
    return s_instance;
}

CubeClient::CubeClient(SOConfig &config, JKRHeap *heap)
    : m_config(config)
    , m_allocator(heap)
    , m_platform(m_allocator, *CubeRandom::Instance(), CubeNetwork::Instance(),
              *CubeDNS::Instance(), m_socket, *CubeServerManager::Instance(), ClientK::Get()) {
    reset();
}

bool CubeClient::updateState(ClientState &nextState) {
    bool hasChanged = &nextState != m_state.get();
    if (hasChanged) {
        m_state.reset(&nextState);
    }

    if (m_platform.replay || !nextState.needsSockets()) {
        CubeNetwork::Instance().ensureStopped();
    } else {
        m_config.flag = 1 << 0;
        CubeNetwork::Instance().ensureStarted(m_config);
    }

    return hasChanged;
}

CubeClient *CubeClient::s_instance = nullptr;
