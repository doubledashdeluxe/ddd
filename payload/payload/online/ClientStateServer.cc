#include "ClientStateServer.hh"

#include "payload/online/ClientStateError.hh"

#include <common/Log.hh>

ClientStateServer::ClientStateServer(JKRHeap *heap, UDPSocket *socket, Array<u8, 512> *buffer)
    : ClientState(heap), m_socket(socket), m_buffer(buffer), m_index(0) {
    if (!m_socket) {
        m_socket.reset(new (m_heap, 0x20) UDPSocket);
    }
    if (!m_buffer) {
        m_buffer.reset(new (m_heap, 0x20) Array<u8, 512>);
    }
}

ClientStateServer::~ClientStateServer() {}

ClientState &ClientStateServer::read(ClientReadHandler &handler) {
    if (!ServerManager::Instance()->isLocked()) {
        return *this;
    }

    checkConnections();
    checkSocket();

    for (u32 i = 0; i < 16; i++) {
        Socket::Address address;
        s32 result = m_socket->recvFrom(m_buffer->values(), m_buffer->count(), address);
        if (result < 0) {
            break;
        }
        for (u32 j = 0; j < m_connections.count(); j++) {
            m_index = (m_index + 1) % m_connections.count();
            if (m_connections[m_index]->read(*this, m_buffer->values(), result, address)) {
                break;
            }
        }
    }

    if (!handler.clientStateServer()) {
        return *(new (m_heap, 0x4) ClientStateError(m_heap));
    }

    return *this;
}

ClientState &ClientStateServer::writeStateServer() {
    if (!ServerManager::Instance()->isLocked()) {
        return *this;
    }

    checkConnections();
    checkSocket();

    u32 size = m_buffer->count();
    Socket::Address address;
    if (m_connections[m_index]->write(*this, m_buffer->values(), size, address)) {
        m_socket->sendTo(m_buffer->values(), size, address);
    }
    m_index = (m_index + 1) % m_connections.count();

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
        ServerManager *serverManager = ServerManager::Instance();
        for (u32 i = 0; i < serverManager->serverCount(); i++) {
            m_connections.pushBack();
            const ServerManager::Server &server = serverManager->server(i);
            Array<u8, 32> publicK = server.publicKey();
            const char *name = server.address();
            Connection *connection = new (m_heap, 0x4) Connection(m_heap, publicK, name);
            m_connections.back()->reset(connection);
        }
    }
}

void ClientStateServer::checkSocket() {
    if (!m_socket->ok()) {
        for (u32 i = 0; i < m_connections.count(); i++) {
            m_connections[i]->reset();
        }
        m_socket->open(Socket::Domain::IPv4);
    }
}
