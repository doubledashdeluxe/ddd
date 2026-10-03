#include "ClientStateTeam.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStatePack.hh"
#include "portable/online/ClientStatePoll.hh"

ClientStateTeam::ClientStateTeam(const ClientPlatform &platform, ClientState &state,
        const ClientStateTeamWriteInfo &writeInfo)
    : ClientState(platform, &state)
    , m_writeInfo(writeInfo) {
    m_readInfo.ok = true;
}

ClientStateTeam::~ClientStateTeam() {}

bool ClientStateTeam::needsSockets() {
    return true;
}

ClientState &ClientStateTeam::read(ClientReadHandler &handler) {
    ClientState::read(*this);

    if (!handler.clientStateTeam(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateTeam::write(const ClientStateModeWriteInfo &writeInfo) {
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, *this, playerCount));
}

ClientState &ClientStateTeam::write(const ClientStatePackWriteInfo &writeInfo) {
    return *(new (m_platform.allocator) ClientStatePack(m_platform, *this, writeInfo));
}

ClientState &ClientStateTeam::write(const ClientStateTeamWriteInfo &writeInfo) {
    m_writeInfo.kartCount = writeInfo.kartCount;
    m_writeInfo.kartTeams = writeInfo.kartTeams;
    m_writeInfo.entryIndex = writeInfo.entryIndex;
    m_writeInfo.teamCount = writeInfo.teamCount;
    m_writeInfo.continuing = writeInfo.continuing;

    ClientState::write(*this);

    return *this;
}

ClientState &ClientStateTeam::write(const ClientStatePollWriteInfo &writeInfo) {
    return *(new (m_platform.allocator) ClientStatePoll(m_platform, *this, writeInfo));
}

ServerStateServerReader<void> *ClientStateTeam::serverReader() {
    return nullptr;
}

ServerStateUpdateReader<void> *ClientStateTeam::updateReader() {
    return nullptr;
}

ServerStateModeReader<void> *ClientStateTeam::modeReader() {
    return nullptr;
}

ServerStatePackReader<void> *ClientStateTeam::packReader() {
    return nullptr;
}

ServerStateRoomReader<void> *ClientStateTeam::roomReader() {
    return nullptr;
}

ServerStateTeamReader<ClientStateTeam> *ClientStateTeam::teamReader() {
    return this;
}

ServerStatePollReader<void> *ClientStateTeam::pollReader() {
    return nullptr;
}

ServerStateRaceReader<void> *ClientStateTeam::raceReader() {
    return nullptr;
}

ServerTeamStateReader<ClientStateTeam> *ClientStateTeam::serverTeamStateReader() {
    return this;
}

ServerTeamStateMainReader<ClientStateTeam> *ClientStateTeam::mainReader() {
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

ClientStateTeamWriter<ClientStateTeam> &ClientStateTeam::teamWriter() {
    return *this;
}

ClientTeamStateWriter<ClientStateTeam> &ClientStateTeam::clientTeamStateWriter() {
    if (m_writeInfo.isHost) {
        return Upcast<ClientTeamStateWriter::Host>(*this);
    } else {
        return Upcast<ClientTeamStateWriter::Guest>(*this);
    }
}

ClientTeamStateHostWriter<ClientStateTeam> &ClientStateTeam::hostWriter() {
    return *this;
}

ClientTeamStateGuestWriter<ClientStateTeam> &ClientStateTeam::guestWriter() {
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
