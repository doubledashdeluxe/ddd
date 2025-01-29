#include "ClientState.hh"

#include "payload/online/ClientStateError.hh"

ClientState::ClientState(JKRHeap *heap) : m_heap(heap) {}

ClientState::~ClientState() {}

ClientState &ClientState::writeStateIdle() {
    return *(new (m_heap, 0x4) ClientStateError(m_heap));
}

ClientState &ClientState::writeStateServer() {
    return *(new (m_heap, 0x4) ClientStateError(m_heap));
}

ClientState &ClientState::writeStateRoom() {
    return *(new (m_heap, 0x4) ClientStateError(m_heap));
}

ClientState &ClientState::writeStateError() {
    return *(new (m_heap, 0x4) ClientStateError(m_heap));
}
