#include "ClientStateServer.hh"

#include "payload/online/ClientStateError.hh"

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
            u32 index = (m_index + j) % m_connections.count();
            if (m_connections[index]->read(m_buffer->values(), result, address)) {
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
    if (m_connections[m_index]->write(m_buffer->values(), size, address)) {
        m_socket->sendTo(m_buffer->values(), size, address);
    }
    m_index = (m_index + 1) % m_connections.count();

    return *this;
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
