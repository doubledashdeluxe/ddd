#include "CubeClient.hh"

#include "payload/crypto/CubeRandom.hh"
#include "payload/network/CubeDNS.hh"
#include "payload/network/CubeNetwork.hh"
#include "payload/online/ClientK.hh"
#include "payload/online/CubeServerManager.hh"

#include <jsystem/JKRExpHeap.hh>
#include <portable/online/ClientStateIdle.hh>

void CubeClient::reset() {
    updateState(*(new (m_platform.allocator) ClientStateIdle(m_platform)));
}

void CubeClient::read(ClientReadHandler &handler) {
    while (updateState(m_state->read(handler))) {}
}

void CubeClient::writeStateIdle() {
    while (updateState(m_state->writeStateIdle())) {}
}

void CubeClient::writeStateServer(const ClientStateServerWriteInfo &writeInfo) {
    while (updateState(m_state->writeStateServer(writeInfo))) {}
}

void CubeClient::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    while (updateState(m_state->writeStateMode(writeInfo))) {}
}

void CubeClient::writeStatePack(const ClientStatePackWriteInfo &writeInfo) {
    while (updateState(m_state->writeStatePack(writeInfo))) {}
}

void CubeClient::writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) {
    while (updateState(m_state->writeStateRoom(writeInfo))) {}
}

void CubeClient::writeStateTeam(const ClientStateTeamWriteInfo &writeInfo) {
    while (updateState(m_state->writeStateTeam(writeInfo))) {}
}

void CubeClient::writeStatePoll(const ClientStatePollWriteInfo &writeInfo) {
    while (updateState(m_state->writeStatePoll(writeInfo))) {}
}

void CubeClient::writeStateError() {
    while (updateState(m_state->writeStateError())) {}
}

void CubeClient::Init(JKRHeap *parentHeap, SOConfig &config) {
    JKRHeap *heap = JKRExpHeap::Create(16 * 1024, parentHeap, false);
    s_instance = new (heap, 0x4) CubeClient(config, heap);
}

CubeClient *CubeClient::Instance() {
    return s_instance;
}

CubeClient::CubeClient(SOConfig &config, JKRHeap *heap)
    : m_config(config), m_allocator(heap),
      m_platform(m_allocator, *CubeRandom::Instance(), CubeNetwork::Instance(),
              *CubeDNS::Instance(), m_socket, *CubeServerManager::Instance(), ClientK::Get()) {
    reset();
}

bool CubeClient::updateState(ClientState &nextState) {
    bool hasChanged = &nextState != m_state.get();
    if (hasChanged) {
        m_state.reset(&nextState);
    }

    if (nextState.needsSockets()) {
        m_config.flag = 1 << 0;
        CubeNetwork::Instance().ensureStarted(m_config);
    } else {
        CubeNetwork::Instance().ensureStopped();
    }

    return hasChanged;
}

CubeClient *CubeClient::s_instance = nullptr;
