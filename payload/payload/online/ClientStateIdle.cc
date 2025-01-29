#include "ClientStateIdle.hh"

#include "payload/online/ClientStateError.hh"
#include "payload/online/ClientStateServer.hh"

ClientStateIdle::ClientStateIdle(JKRHeap *heap) : ClientState(heap) {}

ClientStateIdle::~ClientStateIdle() {}

bool ClientStateIdle::needsSockets() {
    return false;
}

ClientState &ClientStateIdle::read(ClientReadHandler &handler) {
    if (!handler.clientStateIdle()) {
        return *(new (m_heap, 0x4) ClientStateError(m_heap));
    }

    return *this;
}

ClientState &ClientStateIdle::writeStateIdle() {
    return *this;
}

ClientState &ClientStateIdle::writeStateServer() {
    return *(new (m_heap, 0x4) ClientStateServer(m_heap, nullptr));
}
