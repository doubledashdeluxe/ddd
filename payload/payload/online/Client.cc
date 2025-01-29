#include "Client.hh"

#include "payload/network/Socket.hh"
#include "payload/online/ClientStateIdle.hh"

#include <common/Arena.hh>
#include <jsystem/JKRExpHeap.hh>

void Client::reset() {
    updateState(*(new (m_heap, 0x4) ClientStateIdle(m_heap)));
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

void Client::Init(SOConfig &config) {
    size_t heapSize = 16 * 1024;
    void *heapPtr = MEM1Arena::Instance()->alloc(heapSize, 0x4);
    JKRHeap *heap = JKRExpHeap::Create(heapPtr, heapSize, JKRHeap::GetRootHeap(), false);
    s_instance = new (heap, 0x4) Client(config, heap);
}

Client *Client::Instance() {
    return s_instance;
}

Client::Client(SOConfig &config, JKRHeap *heap) : m_config(config), m_heap(heap) {
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
