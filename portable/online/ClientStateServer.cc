#include "ClientStateServer.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"

#include <formats/Version.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

ClientStateServer::ClientStateServer(const ClientPlatform &platform) : ClientState(platform) {
    m_platform.socket.close();
    snprintf(m_version.values(), m_version.count(), "%s", Version);
}

ClientStateServer::~ClientStateServer() {}

bool ClientStateServer::needsSockets() {
    return true;
}

ClientState &ClientStateServer::read(ClientReadHandler &handler) {
    if (!m_platform.serverManager.isLocked()) {
        return *this;
    }

    checkConnections();
    checkServers();

    m_readInfo.networkIsRunning = m_platform.network.isRunning();
    if (m_readInfo.networkIsRunning) {
        ClientState::read(*this);
    }

    m_readInfo.networkName = m_platform.network.name();
    m_readInfo.networkAddress = m_readInfo.networkIsRunning ? m_platform.network.address() : 0;
    for (u32 i = 0; i < m_connections.count(); i++) {
        ReadInfo::Server &server = m_readInfo.servers[i];
        server.address = m_connections[i]->address();
        server.versionIsCompatible = server.protocolVersion == ProtocolVersion;
    }

    if (!handler.clientStateServer(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateServer::writeStateServer(const WriteInfo &writeInfo) {
    if (!m_platform.serverManager.isLocked()) {
        return *this;
    }

    checkConnections();
    checkServers();

    m_readInfo.networkIsRunning = m_platform.network.isRunning();
    if (m_readInfo.networkIsRunning) {
        m_writeInfo = &writeInfo;

        ClientState::write(*this);
    }

    return *this;
}

ClientState &ClientStateServer::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connections[writeInfo.serverIndex].release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ServerStateServerReader<ClientStateServer> *ClientStateServer::serverReader() {
    return this;
}

ServerStateModeReader<void> *ClientStateServer::modeReader() {
    return nullptr;
}

ServerStatePackReader<void> *ClientStateServer::packReader() {
    return nullptr;
}

ServerStateRoomReader<void> *ClientStateServer::roomReader() {
    return nullptr;
}

ServerStateTeamReader<void> *ClientStateServer::teamReader() {
    return nullptr;
}

ServerStatePollReader<void> *ClientStateServer::pollReader() {
    return nullptr;
}

ServerStateRaceReader<void> *ClientStateServer::raceReader() {
    return nullptr;
}

bool ClientStateServer::isProtocolVersionValid(u32 /* protocolVersion */) {
    return true;
}

void ClientStateServer::setProtocolVersion(u32 protocolVersion) {
    m_readInfo.servers[m_readIndex].protocolVersion = protocolVersion;
}

bool ClientStateServer::isVersionCountValid(u32 /* versionCount */) {
    return true;
}

void ClientStateServer::setVersionCount(u32 versionCount) {
    m_readInfo.servers[m_readIndex].version.getOrEmplace()[versionCount] = '\0';
}

bool ClientStateServer::isVersionElementValid(u32 /* i0 */, u8 versionElement) {
    return versionElement != '\0';
}

void ClientStateServer::setVersionElement(u32 i0, u8 versionElement) {
    m_readInfo.servers[m_readIndex].version.getOrEmplace()[i0] = versionElement;
}

ServerIdentityReader<ClientStateServer> *ClientStateServer::serverIdentityReader() {
    return this;
}

ServerIdentityUnspecifiedReader<ClientStateServer> *ClientStateServer::unspecifiedReader() {
    return this;
}

ServerIdentitySpecifiedReader<ClientStateServer> *ClientStateServer::specifiedReader() {
    if (m_readInfo.servers[m_writeIndex].versionIsCompatible) {
        return this;
    }
    return nullptr;
}

bool ClientStateServer::isMotdCountValid(u32 /* motdCount */) {
    return true;
}

void ClientStateServer::setMotdCount(u32 motdCount) {
    m_readInfo.servers[m_readIndex].motd.getOrEmplace()[motdCount] = '\0';
}

bool ClientStateServer::isMotdElementValid(u32 /* i0 */, u8 motdElement) {
    return motdElement != '\0';
}

void ClientStateServer::setMotdElement(u32 i0, u8 motdElement) {
    m_readInfo.servers[m_readIndex].motd.getOrEmplace()[i0] = motdElement;
}

bool ClientStateServer::isPlayerCountValid(u16 /* playerCount */) {
    return true;
}

void ClientStateServer::setPlayerCount(u16 playerCount) {
    m_readInfo.servers[m_readIndex].playerCount = playerCount;
}

ClientStateServerWriter<ClientStateServer> &ClientStateServer::serverWriter() {
    return *this;
}

u32 ClientStateServer::getProtocolVersion() {
    return ProtocolVersion;
}

u32 ClientStateServer::getVersionCount() {
    return strlen(m_version.values());
}

u8 ClientStateServer::getVersionElement(u32 i0) {
    return m_version[i0];
}

ClientIdentityWriter<ClientStateServer> &ClientStateServer::clientIdentityWriter() {
    if (m_readInfo.servers[m_writeIndex].versionIsCompatible) {
        return Upcast<ClientIdentityWriter::Specified>(*this);
    }
    return Upcast<ClientIdentityWriter::Unspecified>(*this);
}

ClientIdentityUnspecifiedWriter<ClientStateServer> &ClientStateServer::unspecifiedWriter() {
    return *this;
}

ClientIdentitySpecifiedWriter<ClientStateServer> &ClientStateServer::specifiedWriter() {
    return *this;
}

u8 ClientStateServer::getFrameRate() {
    return m_writeInfo->frameRate;
}

u32 ClientStateServer::getPlayersCount() {
    return m_writeInfo->playerCount;
}

ClientPlayerWriter<ClientStateServer> &ClientStateServer::playersElementWriter(u32 i0) {
    m_playerIndex = i0;
    return *this;
}

u8 ClientStateServer::getKartCount() {
    return m_writeInfo->kartCount;
}

u8 ClientStateServer::getProfile() {
    return m_writeInfo->players[m_playerIndex].profile;
}

u8 ClientStateServer::getNameElement(u32 i0) {
    return m_writeInfo->players[m_playerIndex].name[i0];
}

void ClientStateServer::checkConnections() {
    if (m_connections.empty()) {
        for (u32 i = 0; i < m_platform.serverManager.serverCount(); i++) {
            const ServerManager::Server &server = m_platform.serverManager.server(i);
            const PublicKey &publicK = server.publicKey();
            const DNS::Name &name = server.address();
            Optional<u16> port = server.port();
            Connection *connection =
                    new (m_platform.allocator) Connection(m_platform, publicK, name, port);
            m_connections.emplaceBack()->reset(connection);
        }
    }
}

void ClientStateServer::checkServers() {
    if (m_readInfo.servers.empty()) {
        for (u32 i = 0; i < m_platform.serverManager.serverCount(); i++) {
            m_readInfo.servers.emplaceBack()->versionIsCompatible = false;
        }
    }
}
