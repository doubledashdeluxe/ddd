#include "ClientStateRace.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStatePoll.hh"

ClientStateRace::ClientStateRace(const ClientPlatform &platform, Connection &connection,
        const ClientStateRaceWriteInfo &writeInfo)
    : ClientState(platform), m_writeInfo(writeInfo) {
    m_connections.pushBack();
    m_connections.back()->reset(&connection);
    m_readInfo.ok = true;
}

ClientStateRace::~ClientStateRace() {}

bool ClientStateRace::needsSockets() {
    return true;
}

ClientState &ClientStateRace::read(ClientReadHandler &handler) {
    ClientState::read(*this);

    if (!handler.clientStateRace(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateRace::writeStatePoll(const ClientStatePollWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStatePoll(m_platform, connection, writeInfo));
}

ClientState &ClientStateRace::writeStateRace(const ClientStateRaceWriteInfo &writeInfo) {
    m_writeInfo.frame = writeInfo.frame;

    ClientState::write(*this);

    return *this;
}

ServerStateServerReader *ClientStateRace::serverReader() {
    return nullptr;
}

ServerStateModeReader *ClientStateRace::modeReader() {
    return nullptr;
}

ServerStatePackReader *ClientStateRace::packReader() {
    return nullptr;
}

ServerStateRoomReader *ClientStateRace::roomReader() {
    return nullptr;
}

ServerStateTeamReader *ClientStateRace::teamReader() {
    return nullptr;
}

ServerStatePollReader *ClientStateRace::pollReader() {
    return nullptr;
}

ServerStateRaceReader *ClientStateRace::raceReader() {
    return this;
}

ServerRaceStateReader *ClientStateRace::serverRaceStateReader() {
    return this;
}

ServerRaceStateMainReader *ClientStateRace::mainReader() {
    return this;
}

bool ClientStateRace::isErrorValid() {
    return true;
}

void ClientStateRace::setError() {
    m_readInfo.ok = false;
}

bool ClientStateRace::isFrameValid(u16 frame) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || frame > info->frame;
}

void ClientStateRace::setFrame(u16 frame) {
    m_readInfo.info.getOrEmplace().frame = frame;
}

bool ClientStateRace::isClientFrameValid(u16 clientFrame) {
    if (clientFrame <= m_writeInfo.frame) {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || clientFrame >= info->clientFrame;
    } else {
        return false;
    }
}

void ClientStateRace::setClientFrame(u16 clientFrame) {
    m_readInfo.info.getOrEmplace().clientFrame = clientFrame;
}

ClientStateRaceWriter &ClientStateRace::raceWriter() {
    return *this;
}

u16 ClientStateRace::getFrame() {
    return m_writeInfo.frame;
}
