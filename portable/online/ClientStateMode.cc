#include "ClientStateMode.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStatePack.hh"
#include "portable/online/ClientStateRoom.hh"
#include "portable/online/ClientStateServer.hh"

ClientStateMode::ClientStateMode(const ClientPlatform &platform, Connection &connection,
        u8 playerCount)
    : ClientState(platform)
    , m_playerCount(playerCount) {
    m_connections.pushBack();
    m_connections.back()->reset(&connection);
}

ClientStateMode::~ClientStateMode() {}

bool ClientStateMode::needsSockets() {
    return true;
}

ClientState &ClientStateMode::read(ClientReadHandler &handler) {
    ClientState::read(*this);

    if (!handler.clientStateMode(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateMode::writeStateServer(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateServer(m_platform));
}

ClientState &ClientStateMode::writeStateMode(const WriteInfo & /* writeInfo */) {
    ClientState::write(*this);

    return *this;
}

ClientState &ClientStateMode::writeStatePack(const ClientStatePackWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStatePack(m_platform, connection, writeInfo));
}

ClientState &ClientStateMode::writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStateRoom(m_platform, connection, writeInfo));
}

ServerStateServerReader *ClientStateMode::serverReader() {
    return nullptr;
}

ServerStateModeReader *ClientStateMode::modeReader() {
    return this;
}

ServerStatePackReader *ClientStateMode::packReader() {
    return nullptr;
}

ServerStateRoomReader *ClientStateMode::roomReader() {
    return nullptr;
}

ServerStateTeamReader *ClientStateMode::teamReader() {
    return nullptr;
}

ServerStatePollReader *ClientStateMode::pollReader() {
    return nullptr;
}

ServerStateRaceReader *ClientStateMode::raceReader() {
    return nullptr;
}

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
