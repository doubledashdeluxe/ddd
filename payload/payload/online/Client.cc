#include "Client.hh"

#include "payload/online/ClientStateServer.hh"

#include <common/Arena.hh>
#include <jsystem/JKRExpHeap.hh>

void Client::reset() {
    m_state.reset(new (m_heap, 0x4) ClientStateServer(m_heap, nullptr, nullptr));
}

void Client::read(ClientReadHandler &handler) {
    while (updateState(m_state->read(handler))) {}
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

void Client::Init() {
    size_t heapSize = 0x20000;
    void *heapPtr = MEM2Arena::Instance()->alloc(heapSize, 0x4);
    JKRHeap *heap = JKRExpHeap::Create(heapPtr, heapSize, JKRHeap::GetRootHeap(), false);
    s_instance = new (heap, 0x4) Client(heap);
}

Client *Client::Instance() {
    return s_instance;
}

Client::Client(JKRHeap *heap) : m_heap(heap) {
    reset();
}

bool Client::updateState(ClientState &nextState) {
    if (&nextState == m_state.get()) {
        return false;
    }

    m_state.reset(&nextState);
    return true;
}

Client *Client::s_instance = nullptr;
