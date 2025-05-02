#include "Client.hh"

#include "payload/network/Socket.hh"
#include "payload/online/ClientStateIdle.hh"

#include <jsystem/JKRExpHeap.hh>

void Client::reset() {
    updateState(*(new (m_platform.allocator()) ClientStateIdle(m_platform)));
}

void Client::read(ClientReadHandler &handler) {
    while (updateState(m_state->read(handler))) {}
}

void Client::writeStateIdle() {
    while (updateState(m_state->writeStateIdle())) {}
}

void Client::writeStateServer() {
    while (updateState(m_state->writeStateServer())) {}
}

void Client::writeStateRoom() {
    while (updateState(m_state->writeStateRoom())) {}
}

void Client::writeStateError() {
    while (updateState(m_state->writeStateError())) {}
}

void Client::Init(JKRHeap *parentHeap, SOConfig &config) {
    JKRHeap *heap = JKRExpHeap::Create(16 * 1024, parentHeap, false);
    s_instance = new (heap, 0x4) Client(config, heap);
}

Client *Client::Instance() {
    return s_instance;
}

Client::Client(SOConfig &config, JKRHeap *heap) : m_config(config), m_platform(heap) {
    reset();
}

bool Client::updateState(ClientState &nextState) {
    bool hasChanged = &nextState != m_state.get();
    if (hasChanged) {
        m_state.reset(&nextState);
    }

    if (nextState.needsSockets()) {
        m_config.flag = 1 << 0;
        Socket::EnsureStarted(m_config);
    } else {
        Socket::EnsureStopped();
    }

    return hasChanged;
}

Client *Client::s_instance = nullptr;
