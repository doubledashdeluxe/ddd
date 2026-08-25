#include "ClientStateServer.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStateUpdate.hh"

#include <formats/Version.hh>

extern "C" {
#include <string.h>
}

ClientStateServer::ClientStateServer(const ClientPlatform &platform)
    : ClientState(platform)
    , m_raceCourseOffset(0)
    , m_battleCourseOffset(0) {
    m_platform.socket.close();
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
        server.updateIsAvailable = UpdateIsAvailable(server);
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

        if (m_writeIndex == 0) {
            m_raceCourseOffset = (m_raceCourseOffset + 12) % writeInfo.raceCourses.count();
            m_battleCourseOffset = (m_battleCourseOffset + 12) % writeInfo.battleCourses.count();
        }
    }

    return *this;
}

ClientState &ClientStateServer::writeStateUpdate(const ClientStateUpdateWriteInfo &writeInfo) {
    Connection &connection = *m_connections[writeInfo.serverIndex].release();
    m_connections.reset();
    const Array<char, MaxVersionLength + 1> &version =
            *m_readInfo.servers[writeInfo.serverIndex].version;
    return *(new (m_platform.allocator)
                    ClientStateUpdate(m_platform, connection, version, writeInfo));
}

ClientState &ClientStateServer::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connections[writeInfo.serverIndex].release();
    m_connections.reset();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ServerStateServerReader<ClientStateServer> *ClientStateServer::serverReader() {
    return this;
}

ServerStateUpdateReader<void> *ClientStateServer::updateReader() {
    return nullptr;
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

bool ClientStateServer::isUpdateVersionValid(u8 /* updateVersion */) {
    return true;
}

void ClientStateServer::setUpdateVersion(u8 updateVersion) {
    m_readInfo.servers[m_readIndex].updateVersion = updateVersion;
}

bool ClientStateServer::isReservedValid(u8 /* reserved */) {
    return true;
}

void ClientStateServer::setReserved(u8 /* reserved */) {}

bool ClientStateServer::isProtocolVersionValid(u16 /* protocolVersion */) {
    return true;
}

void ClientStateServer::setProtocolVersion(u16 protocolVersion) {
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
    if (m_readInfo.servers[m_readIndex].versionIsCompatible) {
        return this;
    }
    return nullptr;
}

bool ClientStateServer::isCourseCountValid(u16 /* courseCount */) {
    return true;
}

void ClientStateServer::setCourseCount(u16 courseCount) {
    m_readInfo.servers[m_readIndex].courseCount = courseCount;
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

u8 ClientStateServer::getUpdateVersion() {
    return UpdateVersion;
}

u8 ClientStateServer::getReserved() {
    return 0;
}

u16 ClientStateServer::getProtocolVersion() {
    return ProtocolVersion;
}

u32 ClientStateServer::getVersionCount() {
    return strlen(Version);
}

u8 ClientStateServer::getVersionElement(u32 i0) {
    return Version[i0];
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

u8 ClientStateServer::getRegion() {
    return m_writeInfo->region;
}

u32 ClientStateServer::getPlatformCount() {
    return strlen(m_writeInfo->platform.values());
}

u8 ClientStateServer::getPlatformElement(u32 i0) {
    return m_writeInfo->platform[i0];
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

u32 ClientStateServer::getRaceCoursesCount() {
    return Min<u32>(m_writeInfo->raceCourses.count() - m_raceCourseOffset, 12);
}

u8 ClientStateServer::getRaceCoursesElement(u32 i0, u32 i1) {
    return m_writeInfo->raceCourses[i0 + m_raceCourseOffset][i1];
}

u8 ClientStateServer::getRaceCourseOffset() {
    return m_raceCourseOffset;
}

u32 ClientStateServer::getBattleCoursesCount() {
    return Min<u32>(m_writeInfo->battleCourses.count() - m_battleCourseOffset, 12);
}

u8 ClientStateServer::getBattleCoursesElement(u32 i0, u32 i1) {
    return m_writeInfo->battleCourses[i0 + m_battleCourseOffset][i1];
}

u8 ClientStateServer::getBattleCourseOffset() {
    return m_battleCourseOffset;
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
            m_readInfo.servers.emplaceBack()->updateIsAvailable = false;
        }
    }
}

bool ClientStateServer::UpdateIsAvailable(const ReadInfo::Server &server) {
    if (server.updateVersion != UpdateVersion) {
        return false;
    }

    if (!server.protocolVersion || !server.version) {
        return false;
    }

    if (*server.protocolVersion != ProtocolVersion) {
        return true;
    }

    if (strcmp(server.version->values(), Version)) {
        return true;
    }

    return false;
}
