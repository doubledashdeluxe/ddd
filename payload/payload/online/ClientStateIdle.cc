#include "ClientStateIdle.hh"

#include "payload/online/ClientStateError.hh"
#include "payload/online/ClientStateServer.hh"

ClientStateIdle::ClientStateIdle(Allocator &allocator) : ClientState(allocator) {}

ClientStateIdle::~ClientStateIdle() {}

bool ClientStateIdle::needsSockets() {
    return false;
}

ClientState &ClientStateIdle::read(ClientReadHandler &handler) {
    if (!handler.clientStateIdle()) {
        return *(new (m_allocator) ClientStateError(m_allocator));
    }

    return *this;
}

ClientState &ClientStateIdle::writeStateIdle() {
    return *this;
}

ClientState &ClientStateIdle::writeStateServer() {
    return *(new (m_allocator) ClientStateServer(m_allocator, nullptr));
}
