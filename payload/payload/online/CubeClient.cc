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

void CubeClient::write(const ClientStateIdleWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateServerWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateUpdateWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateModeWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStatePackWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateRoomWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateTeamWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStatePollWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateRaceWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
}

void CubeClient::write(const ClientStateErrorWriteInfo &writeInfo) {
    while (updateState(m_state->write(writeInfo))) {}
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

    if (nextState.needsSockets()) {
        m_config.flag = 1 << 0;
        CubeNetwork::Instance().ensureStarted(m_config);
    } else {
        CubeNetwork::Instance().ensureStopped();
    }

    return hasChanged;
}

CubeClient *CubeClient::s_instance = nullptr;
