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

    while (updateState((this->*m_writer)())) {}
    return true;
}

bool FakeClient::clientStateIdle() {
    m_writer = &FakeClient::writeStateServer;
    return true;
}

bool FakeClient::clientStateServer(const ClientStateServerReadInfo &readInfo) {
    const ClientStateServerReadInfo::Server &server = readInfo.servers[0];
    if (server.motd) {
        m_writer = &FakeClient::writeStateMode;
    }
    return true;
}

bool FakeClient::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    m_writer = &FakeClient::writeStatePack;
    return true;
}

bool FakeClient::clientStatePack(const ClientStatePackReadInfo & /* readInfo */) {
    return true;
}

void FakeClient::clientStateError() {
    m_writer = nullptr;
}

ClientState &FakeClient::writeStateServer() {
    ClientStateServerWriteInfo writeInfo;
    writeInfo.playerCount = 1;
    writeInfo.players[0].profile = 0;
    writeInfo.players[0].name = "AAA";
    return m_state->writeStateServer(writeInfo);
}

ClientState &FakeClient::writeStateMode() {
    ClientStateModeWriteInfo writeInfo;
    writeInfo.playerCount = 1;
    writeInfo.serverIndex = 0;
    return m_state->writeStateMode(writeInfo);
}

ClientState &FakeClient::writeStatePack() {
    ClientStatePackWriteInfo writeInfo;
    writeInfo.modeIndex = 0;
    writeInfo.packCount = 1;
    writeInfo.packs[0].hash.fill(0xff);
    return m_state->writeStatePack(writeInfo);
}

bool FakeClient::updateState(ClientState &nextState) {
    bool hasChanged = &nextState != m_state.get();
    if (hasChanged) {
        m_state.reset(&nextState);
    }
    return hasChanged;
}
