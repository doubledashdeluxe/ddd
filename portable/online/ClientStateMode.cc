#include "ClientStateMode.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateServer.hh"

ClientStateMode::ClientStateMode(const ClientPlatform &platform, Connection &connection,
        u8 playerCount)
    : ClientState(platform), m_connection(&connection), m_playerCount(playerCount) {}

ClientStateMode::~ClientStateMode() {}

bool ClientStateMode::needsSockets() {
    return true;
}

ClientState &ClientStateMode::read(ClientReadHandler &handler) {
    checkSocket();

    for (u32 i = 0; i < 16; i++) {
        Array<u8, 512> buffer;
        Address address;
        s32 result = m_platform.socket.recvFrom(buffer.values(), buffer.count(), address);
        if (result < 0) {
            break;
        }
        m_connection->read(*this, buffer.values(), result, address);
    }

    if (!handler.clientStateMode(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateMode::writeStateServer(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateServer(m_platform));
}

ClientState &ClientStateMode::writeStateMode(const WriteInfo & /* writeInfo */) {
    checkSocket();

    Array<u8, 512> buffer;
    u32 size = buffer.count();
    Address address;
    if (m_connection->write(*this, buffer.values(), size, address)) {
        m_platform.socket.sendTo(buffer.values(), size, address);
    }

    return *this;
}

ServerStateServerReader *ClientStateMode::serverReader() {
    return nullptr;
}

ServerStateModeReader *ClientStateMode::modeReader() {
    return this;
}

bool ClientStateMode::isModesCountValid(u32 /* modesCount */) {
    return true;
}

void ClientStateMode::setModesCount(u32 /* modesCount */) {}

ServerModeReader *ClientStateMode::modesElementReader(u32 i0) {
    m_modeIndex = i0;
    return this;
}

bool ClientStateMode::isMmrsCountValid(u32 mmrsCount) {
    return mmrsCount == m_playerCount;
}

void ClientStateMode::setMmrsCount(u32 /* mmrsCount */) {}

bool ClientStateMode::isMmrsElementValid(u32 /* i0 */, u16 /* mmrsElement */) {
    return true;
}

void ClientStateMode::setMmrsElement(u32 i0, u16 mmrsElement) {
    m_readInfo.modes.emplace()[m_modeIndex].mmrs[i0] = mmrsElement;
}

bool ClientStateMode::isPlayerCountValid(u16 /* playerCount */) {
    return true;
}

void ClientStateMode::setPlayerCount(u16 playerCount) {
    m_readInfo.modes.emplace()[m_modeIndex].playerCount = playerCount;
}

ClientStateModeWriter &ClientStateMode::modeWriter() {
    return *this;
}

void ClientStateMode::checkSocket() {
    if (!m_platform.socket.ok()) {
        m_connection->reset();
        m_platform.socket.open();
    }
}
