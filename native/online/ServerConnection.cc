#include "ServerConnection.hh"

#include "native/online/ServerConnectionStateIdle.hh"

ServerConnection::ServerConnection(const ServerPlatform &platform)
    : m_state(new(platform.allocator) ServerConnectionStateIdle(platform)) {}

ServerConnection::~ServerConnection() = default;

void ServerConnection::transfer(const u8 *input, size_t inputSize, std::vector<u8> &output) {
    output.clear();
    while (updateState(m_state->transfer(input, inputSize, output))) {}
}

bool ServerConnection::updateState(ServerConnectionState &nextState) {
    if (&nextState == m_state.get()) {
        return false;
    }

    m_state.reset(&nextState);
    return true;
}
