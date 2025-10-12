#include "ClientState.hh"

#include "portable/online/ClientStateError.hh"

ClientState::ClientState(const ClientPlatform &platform) : m_platform(platform) {}

ClientState::~ClientState() {}

ClientState &ClientState::writeStateIdle() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateServer(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateMode(const ClientStateModeWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateError() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}
