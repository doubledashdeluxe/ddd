#include "ClientState.hh"

#include "portable/online/ClientStateError.hh"

ClientState::ClientState(const ClientPlatform &platform, ClientState *state)
    : m_platform(platform)
    , m_readIndex(0)
    , m_writeIndex(0) {
    if (state) {
        UniquePtr<Connection> *connection = state->m_connections.front();
        if (connection) {
            m_connections.emplaceBack()->reset(connection->release());
        }
    }
}

ClientState::~ClientState() {}

ClientState &ClientState::write(const ClientStateIdleWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateUpdateWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateModeWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStatePackWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateRoomWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateTeamWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStatePollWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateRaceWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::write(const ClientStateErrorWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

void ClientState::read(ConnectionState::Reader &reader) {
    checkSocket();

    if (m_connections.empty()) {
        return;
    }

    for (u32 i = 0; i < 16; i++) {
        u8 buffer[BufferSize];
        Address address;
        s32 result = m_platform.socket.recvFrom(buffer, Count(buffer), address);
        if (result < 0) {
            break;
        }
        for (u32 j = 0; j < m_connections.count(); j++) {
            m_readIndex = (m_readIndex + 1) % m_connections.count();
            if (m_connections[m_readIndex]->read(reader, buffer, result, address)) {
                break;
            }
        }
    }
}

void ClientState::write(ConnectionState::Writer &writer) {
    checkSocket();

    if (m_connections.empty()) {
        return;
    }

    u8 buffer[BufferSize];
    u32 size = Count(buffer);
    Address address;
    if (m_connections[m_writeIndex]->write(writer, buffer, size, address)) {
        m_platform.socket.sendTo(buffer, size, address);
    }
    m_writeIndex = (m_writeIndex + 1) % m_connections.count();
}

void ClientState::checkSocket() {
    if (m_platform.socket.ok()) {
        return;
    }

    for (u32 i = 0; i < m_connections.count(); i++) {
        m_connections[i]->reset();
    }
    m_platform.socket.open();
}
