#include "ClientStateServer.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"

ClientStateServer::ClientStateServer(const ClientPlatform &platform)
    : ClientState(platform), m_readIndex(0), m_writeIndex(0) {}

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

    m_info.networkIsRunning = m_platform.network.isRunning();
    if (m_info.networkIsRunning) {
        checkSocket();

        if (!m_connections.empty()) {
            for (u32 i = 0; i < 16; i++) {
                Array<u8, 512> buffer;
                Address address;
                s32 result = m_platform.socket.recvFrom(buffer.values(), buffer.count(), address);
                if (result < 0) {
                    break;
                }
                for (u32 j = 0; j < m_connections.count(); j++) {
                    m_readIndex = (m_readIndex + 1) % m_connections.count();
                    if (m_connections[m_readIndex]->read(*this, buffer.values(), result, address)) {
                        ClientStateServerInfo::Server &server = m_info.servers[m_readIndex];
                        u32 *protocolVersion = server.protocolVersion.get();
                        server.versionIsCompatible = protocolVersion && *protocolVersion == 1;
                        break;
                    }
                }
            }
        }
    }

    m_info.networkName = m_platform.network.name();
    m_info.networkAddress = m_info.networkIsRunning ? m_platform.network.address() : 0;
    for (u32 i = 0; i < m_connections.count(); i++) {
        m_info.servers[i].address = m_connections[i]->address();
    }

    if (!handler.clientStateServer(m_info)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateServer::writeStateServer() {
    if (!m_platform.serverManager.isLocked()) {
        return *this;
    }

    checkConnections();
    checkServers();

    m_info.networkIsRunning = m_platform.network.isRunning();
    if (m_info.networkIsRunning) {
        checkSocket();

        if (!m_connections.empty()) {
            Array<u8, 512> buffer;
            u32 size = buffer.count();
            Address address;
            if (m_connections[m_writeIndex]->write(*this, buffer.values(), size, address)) {
                m_platform.socket.sendTo(buffer.values(), size, address);
            }
            m_writeIndex = (m_writeIndex + 1) % m_connections.count();
        }
    }

    return *this;
}

ServerStateServerReader *ClientStateServer::serverReader() {
    return this;
}

ServerStateRoomReader *ClientStateServer::roomReader() {
    return nullptr;
}

bool ClientStateServer::isProtocolVersionValid(u32 /* protocolVersion */) {
    return true;
}

void ClientStateServer::setProtocolVersion(u32 protocolVersion) {
    m_info.servers[m_readIndex].protocolVersion = protocolVersion;
}

ServerVersionReader *ClientStateServer::serverVersionReader() {
    return this;
}

ServerIdentityReader *ClientStateServer::serverIdentityReader() {
    return this;
}

bool ClientStateServer::isMajorValid(u8 /* major */) {
    return true;
}

void ClientStateServer::setMajor(u8 major) {
    m_info.servers[m_readIndex].version.getOrEmplace().major = major;
}

bool ClientStateServer::isMinorValid(u8 /* minor */) {
    return true;
}

void ClientStateServer::setMinor(u8 minor) {
    m_info.servers[m_readIndex].version.getOrEmplace().minor = minor;
}

bool ClientStateServer::isPatchValid(u8 /* patch */) {
    return true;
}

void ClientStateServer::setPatch(u8 patch) {
    m_info.servers[m_readIndex].version.getOrEmplace().patch = patch;
}

ServerIdentityUnspecifiedReader *ClientStateServer::unspecifiedReader() {
    return this;
}

ServerIdentitySpecifiedReader *ClientStateServer::specifiedReader() {
    if (m_info.servers[m_writeIndex].versionIsCompatible) {
        return this;
    }
    return nullptr;
}

bool ClientStateServer::isMotdCountValid(u32 /* motdCount */) {
    return true;
}

void ClientStateServer::setMotdCount(u32 motdCount) {
    m_info.servers[m_readIndex].motd.getOrEmplace()[motdCount] = '\0';
}

bool ClientStateServer::isMotdElementValid(u32 /* i0 */, u8 motdElement) {
    return motdElement != '\0';
}

void ClientStateServer::setMotdElement(u32 i0, u8 motdElement) {
    m_info.servers[m_readIndex].motd.getOrEmplace()[i0] = motdElement;
}

bool ClientStateServer::isPlayerCountValid(u16 /* playerCount */) {
    return true;
}

void ClientStateServer::setPlayerCount(u16 playerCount) {
    m_info.servers[m_readIndex].playerCount = playerCount;
}

ClientStateServerWriter &ClientStateServer::serverWriter() {
    return *this;
}

u32 ClientStateServer::getProtocolVersion() {
    return 1;
}

ClientVersionWriter &ClientStateServer::clientVersionWriter() {
    return *this;
}

ClientIdentityWriter &ClientStateServer::clientIdentityWriter() {
    if (m_info.servers[m_writeIndex].versionIsCompatible) {
        return Upcast<ClientIdentityWriter::Specified>(*this);
    }
    return Upcast<ClientIdentityWriter::Unspecified>(*this);
}

u8 ClientStateServer::getMajor() {
    return 2;
}

u8 ClientStateServer::getMinor() {
    return 3;
}

u8 ClientStateServer::getPatch() {
    return 4;
}

ClientIdentityUnspecifiedWriter &ClientStateServer::unspecifiedWriter() {
    return *this;
}

ClientIdentitySpecifiedWriter &ClientStateServer::specifiedWriter() {
    return *this;
}

u32 ClientStateServer::getPlayersCount() {
    return 1;
}

u32 ClientStateServer::getPlayersCount(u32 /* i0 */) {
    return 1;
}

u32 ClientStateServer::getPlayersCount(u32 /* i0 */, u32 /* i1 */) {
    return 3;
}

u8 ClientStateServer::getPlayersElement(u32 /* i0 */, u32 /* i1 */, u32 /* i2 */) {
    return 'A';
}

void ClientStateServer::checkConnections() {
    if (m_connections.empty()) {
        for (u32 i = 0; i < m_platform.serverManager.serverCount(); i++) {
            m_connections.pushBack();
            const ServerManager::Server &server = m_platform.serverManager.server(i);
            const Array<u8, 32> &publicK = server.publicKey();
            const Array<char, 32> &name = server.address();
            u16 port = server.port();
            Connection *connection =
                    new (m_platform.allocator) Connection(m_platform, publicK, name, port);
            m_connections.back()->reset(connection);
        }
    }
}

void ClientStateServer::checkSocket() {
    if (!m_platform.socket.ok()) {
        for (u32 i = 0; i < m_connections.count(); i++) {
            m_connections[i]->reset();
        }
        m_platform.socket.open();
    }
}

void ClientStateServer::checkServers() {
    if (m_info.servers.empty()) {
        for (u32 i = 0; i < m_platform.serverManager.serverCount(); i++) {
            m_info.servers.pushBack();
            m_info.servers[i].versionIsCompatible = false;
        }
    }
}
