#include "ClientState.hh"

#include "portable/online/ClientStateError.hh"

ClientState::ClientState(const ClientPlatform &platform)
    : m_platform(platform)
    , m_readIndex(0)
    , m_writeIndex(0) {}

ClientState::~ClientState() {}

ClientState &ClientState::writeStateIdle() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateServer(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateMode(const ClientStateModeWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStatePack(const ClientStatePackWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateRoom(const ClientStateRoomWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateTeam(const ClientStateTeamWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStatePoll(const ClientStatePollWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateRace(const ClientStateRaceWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

ClientState &ClientState::writeStateError() {
    return *(new (m_platform.allocator) ClientStateError(m_platform));
}

void ClientState::read(ServerStateReader &reader) {
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
                if (m_connections[m_readIndex]->read(reader, buffer.values(), result, address)) {
                    break;
                }
            }
        }
    }
}

void ClientState::write(ClientStateWriter &writer) {
    checkSocket();

    if (!m_connections.empty()) {
        Array<u8, 512> buffer;
        u32 size = buffer.count();
        Address address;
        if (m_connections[m_writeIndex]->write(writer, buffer.values(), size, address)) {
            m_platform.socket.sendTo(buffer.values(), size, address);
        }
        m_writeIndex = (m_writeIndex + 1) % m_connections.count();
    }
}

void ClientState::checkSocket() {
    if (!m_platform.socket.ok()) {
        for (u32 i = 0; i < m_connections.count(); i++) {
            m_connections[i]->reset();
        }
        m_platform.socket.open();
    }
}
