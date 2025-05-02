#include "ClientState.hh"

#include "payload/online/ClientStateError.hh"

ClientState::ClientState(Allocator &allocator) : m_allocator(allocator) {}

ClientState::~ClientState() {}

ClientState &ClientState::writeStateIdle() {
    return *(new (m_allocator) ClientStateError(m_allocator));
}

ClientState &ClientState::writeStateServer() {
    return *(new (m_allocator) ClientStateError(m_allocator));
}

ClientState &ClientState::writeStateRoom() {
    return *(new (m_allocator) ClientStateError(m_allocator));
}

ClientState &ClientState::writeStateError() {
    return *(new (m_allocator) ClientStateError(m_allocator));
}
