#include "ClientState.hh"

#include "portable/online/ClientStateError.hh"

ClientState::ClientState(const ClientPlatform &platform) : m_platform(platform) {}

ClientState::~ClientState() {}

ClientState &ClientState::writeStateIdle() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateServer() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateRoom() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateError() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}
