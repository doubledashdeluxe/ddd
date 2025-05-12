#include "FakeClient.hh"

#include <portable/online/ClientStateIdle.hh>

FakeClient::FakeClient(const std::map<std::vector<std::string>, u32> &dnsServers,
        const Array<u8, 32> &serverK, const std::vector<u8> &data,
        const Ring<ServerManager::Server, ServerManager::MaxServerCount> &servers,
        const Array<u8, 32> &clientK)
    : m_dnsSocket(dnsServers), m_dns(m_dnsSocket),
      m_serverPlatform(m_allocator, m_random, serverK, data), m_socket(m_serverPlatform),
      m_serverManager(servers),
      m_platform(m_allocator, m_random, m_network, m_dns, m_socket, m_serverManager, clientK),
      m_state(new(m_platform.allocator) ClientStateIdle(m_platform)) {}

FakeClient::~FakeClient() = default;

void FakeClient::read() {
    while (updateState(m_state->read(*this))) {}
}

bool FakeClient::write() {
    if (!m_writer) {
        return false;
    }

    while (updateState(((*m_state).*m_writer)())) {}
    return true;
}

bool FakeClient::clientStateIdle() {
    m_writer = &ClientState::writeStateServer;
    return true;
}

bool FakeClient::clientStateServer() {
    return true;
}

void FakeClient::clientStateError() {
    m_writer = nullptr;
}

bool FakeClient::updateState(ClientState &nextState) {
    bool hasChanged = &nextState != m_state.get();
    if (hasChanged) {
        m_state.reset(&nextState);
    }
    return hasChanged;
}
