#include "ClientStateTeam.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStatePack.hh"

ClientStateTeam::ClientStateTeam(const ClientPlatform &platform, Connection &connection,
        const ClientStateTeamWriteInfo &writeInfo)
    : ClientState(platform), m_connection(&connection), m_writeInfo(writeInfo) {
    m_readInfo.ok = true;
}

ClientStateTeam::~ClientStateTeam() {}

bool ClientStateTeam::needsSockets() {
    return true;
}

ClientState &ClientStateTeam::read(ClientReadHandler &handler) {
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

    if (!handler.clientStateTeam(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateTeam::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connection.release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ClientState &ClientStateTeam::writeStatePack(const ClientStatePackWriteInfo &writeInfo) {
    Connection &connection = *m_connection.release();
    return *(new (m_platform.allocator) ClientStatePack(m_platform, connection, writeInfo));
}

ClientState &ClientStateTeam::writeStateTeam(const ClientStateTeamWriteInfo &writeInfo) {
    checkSocket();

    m_writeInfo.kartCount = writeInfo.kartCount;
    m_writeInfo.kartTeams = writeInfo.kartTeams;
    m_writeInfo.entryIndex = writeInfo.entryIndex;
    m_writeInfo.teamCount = writeInfo.teamCount;
    m_writeInfo.continuing = writeInfo.continuing;

    Array<u8, 512> buffer;
    u32 size = buffer.count();
    Address address;
    if (m_connection->write(*this, buffer.values(), size, address)) {
        m_platform.socket.sendTo(buffer.values(), size, address);
    }

    return *this;
}

ServerStateServerReader *ClientStateTeam::serverReader() {
    return nullptr;
}

ServerStateModeReader *ClientStateTeam::modeReader() {
    return nullptr;
}

ServerStatePackReader *ClientStateTeam::packReader() {
    return nullptr;
}

ServerStateRoomReader *ClientStateTeam::roomReader() {
    return nullptr;
}

ServerStateTeamReader *ClientStateTeam::teamReader() {
    return this;
}

ServerTeamStateReader *ClientStateTeam::serverTeamStateReader() {
    return this;
}

ServerTeamStateMainReader *ClientStateTeam::mainReader() {
    return this;
}

bool ClientStateTeam::isErrorValid() {
    return true;
}

void ClientStateTeam::setError() {
    m_readInfo.ok = false;
}

bool ClientStateTeam::isTeamsCountValid(u32 teamsCount) {
    return teamsCount == m_writeInfo.kartCount;
}

void ClientStateTeam::setTeamsCount(u32 teamsCount) {
    m_readInfo.info.getOrEmplace().kartCount = teamsCount;
}

bool ClientStateTeam::isTeamsElementValid(u32 i0, u8 teamsElement) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    if (info && info->continuing) {
        return teamsElement == info->kartTeams[i0];
    } else {
        return teamsElement < m_writeInfo.teamCount;
    }
}

void ClientStateTeam::setTeamsElement(u32 i0, u8 teamsElement) {
    m_readInfo.info.getOrEmplace().kartTeams[i0] = teamsElement;
}

bool ClientStateTeam::isEntryIndexValid(u8 entryIndex) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || entryIndex == info->entryIndex;
}

void ClientStateTeam::setEntryIndex(u8 entryIndex) {
    m_readInfo.info.getOrEmplace().entryIndex = entryIndex;
}

bool ClientStateTeam::isContinuingValid(u8 continuing) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || continuing;
}

void ClientStateTeam::setContinuing(u8 continuing) {
    m_readInfo.info.getOrEmplace().continuing = continuing;
}

ClientStateTeamWriter &ClientStateTeam::teamWriter() {
    return *this;
}

ClientTeamStateWriter &ClientStateTeam::clientTeamStateWriter() {
    if (m_writeInfo.isHost) {
        return Upcast<ClientTeamStateWriter::Host>(*this);
    } else {
        return Upcast<ClientTeamStateWriter::Guest>(*this);
    }
}

ClientTeamStateHostWriter &ClientStateTeam::hostWriter() {
    return *this;
}

ClientTeamStateGuestWriter &ClientStateTeam::guestWriter() {
    return *this;
}

u32 ClientStateTeam::getTeamsCount() {
    return m_writeInfo.kartCount;
}

u8 ClientStateTeam::getTeamsElement(u32 i0) {
    return m_writeInfo.kartTeams[i0];
}

u8 ClientStateTeam::getEntryIndex() {
    return m_writeInfo.entryIndex;
}

u8 ClientStateTeam::getContinuing() {
    return m_writeInfo.continuing;
}

void ClientStateTeam::checkSocket() {
    if (!m_platform.socket.ok()) {
        m_connection->reset();
        m_platform.socket.open();
    }
}
