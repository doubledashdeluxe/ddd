#include "ClientStateServer.hh"

#include "portable/Log.hh"
#include "portable/online/ClientStateError.hh"

ClientStateServer::ClientStateServer(ClientPlatform &platform)
    : ClientState(platform), m_index(0) {}

ClientStateServer::~ClientStateServer() {}

bool ClientStateServer::needsSockets() {
    return true;
}

ClientState &ClientStateServer::read(ClientReadHandler &handler) {
    if (!m_platform.serverManager.isLocked()) {
        return *this;
    }

    if (!m_platform.network.isRunning()) {
        return *this;
    }

    checkConnections();
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
                m_index = (m_index + 1) % m_connections.count();
                if (m_connections[m_index]->read(*this, buffer.values(), result, address)) {
                    break;
                }
            }
        }
    }

    if (!handler.clientStateServer()) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateServer::writeStateServer() {
    if (!m_platform.serverManager.isLocked()) {
        return *this;
    }

    if (!m_platform.network.isRunning()) {
        return *this;
    }

    checkConnections();
    checkSocket();

    if (!m_connections.empty()) {
        Array<u8, 512> buffer;
        u32 size = buffer.count();
        Address address;
        if (m_connections[m_index]->write(*this, buffer.values(), size, address)) {
            m_platform.socket.sendTo(buffer.values(), size, address);
        }
        m_index = (m_index + 1) % m_connections.count();
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
    DEBUG("protocolVersion %u %u", m_index, protocolVersion);
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
    DEBUG("major %u %u", m_index, major);
}

bool ClientStateServer::isMinorValid(u8 /* minor */) {
    return true;
}

void ClientStateServer::setMinor(u8 minor) {
    DEBUG("minor %u %u", m_index, minor);
}

bool ClientStateServer::isPatchValid(u8 /* patch */) {
    return true;
}

void ClientStateServer::setPatch(u8 patch) {
    DEBUG("patch %u %u", m_index, patch);
}

ServerIdentityUnspecifiedReader *ClientStateServer::unspecifiedReader() {
    return this;
}

ServerIdentitySpecifiedReader *ClientStateServer::specifiedReader() {
    return this;
}

ClientStateServerWriter &ClientStateServer::serverWriter() {
    return *this;
}

u32 ClientStateServer::getProtocolVersion() {
    return 5;
}

ClientVersionWriter &ClientStateServer::clientVersionWriter() {
    return *this;
}

ClientIdentityWriter &ClientStateServer::clientIdentityWriter() {
    return *this;
}

ClientIdentityUnspecifiedWriter &ClientStateServer::unspecifiedWriter() {
    return *this;
}

u8 ClientStateServer::getMajor() {
    return 6;
}

u8 ClientStateServer::getMinor() {
    return 7;
}

u8 ClientStateServer::getPatch() {
    return 8;
}

void ClientStateServer::checkConnections() {
    if (m_connections.empty()) {
        for (u32 i = 0; i < m_platform.serverManager.serverCount(); i++) {
            m_connections.pushBack();
            const ServerManager::Server &server = m_platform.serverManager.server(i);
            Array<u8, 32> publicK = server.publicKey();
            const char *name = server.address();
            Connection *connection =
                    new (m_platform.allocator) Connection(m_platform, publicK, name);
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
