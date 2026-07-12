#include "ClientStatePoll.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStateRace.hh"

ClientStatePoll::ClientStatePoll(const ClientPlatform &platform, Connection &connection,
        const ClientStatePollWriteInfo &writeInfo)
    : ClientState(platform)
    , m_writeInfo(writeInfo) {
    m_connections.emplaceBack()->reset(&connection);
    m_readInfo.ok = true;
}

ClientStatePoll::~ClientStatePoll() {}

bool ClientStatePoll::needsSockets() {
    return true;
}

ClientState &ClientStatePoll::read(ClientReadHandler &handler) {
    ClientState::read(*this);

    if (!handler.clientStatePoll(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStatePoll::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ClientState &ClientStatePoll::writeStatePoll(const ClientStatePollWriteInfo &writeInfo) {
    m_writeInfo.ready = writeInfo.ready;

    ClientState::write(*this);

    return *this;
}

ClientState &ClientStatePoll::writeStateRace(const ClientStateRaceWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStateRace(m_platform, connection, writeInfo));
}

ServerStateServerReader<void> *ClientStatePoll::serverReader() {
    return nullptr;
}

ServerStateUpdateReader<void> *ClientStatePoll::updateReader() {
    return nullptr;
}

ServerStateModeReader<void> *ClientStatePoll::modeReader() {
    return nullptr;
}

ServerStatePackReader<void> *ClientStatePoll::packReader() {
    return nullptr;
}

ServerStateRoomReader<void> *ClientStatePoll::roomReader() {
    return nullptr;
}

ServerStateTeamReader<void> *ClientStatePoll::teamReader() {
    return nullptr;
}

ServerStatePollReader<ClientStatePoll> *ClientStatePoll::pollReader() {
    return this;
}

ServerStateRaceReader<void> *ClientStatePoll::raceReader() {
    return nullptr;
}

ServerPollStateReader<ClientStatePoll> *ClientStatePoll::serverPollStateReader() {
    return this;
}

ServerPollStatePendingReader<ClientStatePoll> *ClientStatePoll::pendingReader() {
    return this;
}

ServerPollStateReadyReader<ClientStatePoll> *ClientStatePoll::readyReader() {
    return this;
}

bool ClientStatePoll::isErrorValid() {
    return true;
}

void ClientStatePoll::setError() {
    m_readInfo.ok = false;
}

bool ClientStatePoll::isKartIndicesCountValid(u32 kartIndicesCount) {
    return kartIndicesCount >= m_readInfo.kartIndices.count() &&
            kartIndicesCount <= m_writeInfo.kartCount;
}

void ClientStatePoll::setKartIndicesCount(u32 kartIndicesCount) {
    Ring<u8, MaxRoomKartCount> &kartIndices = m_readInfo.kartIndices;
    while (kartIndices.count() < kartIndicesCount) {
        kartIndices.emplaceBack();
    }
}

bool ClientStatePoll::isKartIndicesElementValid(u32 i0, u8 kartIndicesElement) {
    const Ring<u8, MaxRoomKartCount> &kartIndices = m_readInfo.kartIndices;
    if (i0 < kartIndices.count()) {
        return kartIndicesElement == kartIndices[i0];
    } else {
        return kartIndicesElement < m_writeInfo.kartCount;
    }
}

void ClientStatePoll::setKartIndicesElement(u32 i0, u8 kartIndicesElement) {
    m_readInfo.kartIndices[i0] = kartIndicesElement;
}

bool ClientStatePoll::isKartsCountValid(u32 kartsCount) {
    return kartsCount == m_writeInfo.kartCount;
}

void ClientStatePoll::setKartsCount(u32 kartsCount) {
    Ring<u8, MaxRoomKartCount> &kartIndices = m_readInfo.kartIndices;
    while (kartIndices.count() < kartsCount) {
        kartIndices.emplaceBack();
    }
    m_readInfo.ready.emplace().kartCount = kartsCount;
}

ServerPollKartReader<ClientStatePoll> *ClientStatePoll::kartsElementReader(u32 i0) {
    m_kartIndex = i0;
    return this;
}

bool ClientStatePoll::isSelectedKartIndexValid(u8 selectedKartIndex) {
    return selectedKartIndex < m_writeInfo.kartCount;
}

void ClientStatePoll::setSelectedKartIndex(u8 selectedKartIndex) {
    m_readInfo.ready.emplace().kartIndex = selectedKartIndex;
}

bool ClientStatePoll::isKartIndexValid(u8 kartIndex) {
    const Ring<u8, MaxRoomKartCount> &kartIndices = m_readInfo.kartIndices;
    if (m_kartIndex < kartIndices.count()) {
        return kartIndex == kartIndices[m_kartIndex];
    } else {
        return kartIndex < m_writeInfo.kartCount;
    }
}

void ClientStatePoll::setKartIndex(u8 kartIndex) {
    m_readInfo.kartIndices[m_kartIndex] = kartIndex;
}

bool ClientStatePoll::isCharacterIdsElementValid(u32 /* i0 */, u8 /* characterIdsElement */) {
    return true;
}

void ClientStatePoll::setCharacterIdsElement(u32 i0, u8 characterIdsElement) {
    m_readInfo.ready.emplace().karts[m_kartIndex].characterIDs[i0] = characterIdsElement;
}

bool ClientStatePoll::isKartIdValid(u8 /* kartId */) {
    return true;
}

void ClientStatePoll::setKartId(u8 kartId) {
    m_readInfo.ready.emplace().karts[m_kartIndex].kartID = kartId;
}

bool ClientStatePoll::isCourseIndexValid(u8 courseIndex) {
    return courseIndex < m_writeInfo.packCourseCount;
}

void ClientStatePoll::setCourseIndex(u8 courseIndex) {
    m_readInfo.ready.emplace().karts[m_kartIndex].courseIndex = courseIndex;
}

ClientStatePollWriter<ClientStatePoll> &ClientStatePoll::pollWriter() {
    return *this;
}

ClientPollStateWriter<ClientStatePoll> &ClientStatePoll::clientPollStateWriter() {
    if (m_writeInfo.ready) {
        return Upcast<ClientPollStateWriter::Ready>(*this);
    } else {
        return Upcast<ClientPollStateWriter::Pending>(*this);
    }
}

void ClientStatePoll::getPending() {}

ClientPollStateReadyWriter<ClientStatePoll> &ClientStatePoll::readyWriter() {
    return *this;
}

u32 ClientStatePoll::getKartsCount() {
    return m_writeInfo.ready->kartCount;
}

ClientPollKartWriter<ClientStatePoll> &ClientStatePoll::kartsElementWriter(u32 i0) {
    m_kartIndex = i0;
    return *this;
}

ClientCourseIndexWriter<ClientStatePoll> &ClientStatePoll::courseIndexWriter() {
    if (m_writeInfo.ready->courseIndex) {
        return Upcast<ClientCourseIndexWriter::Specified>(*this);
    } else {
        return Upcast<ClientCourseIndexWriter::Unspecified>(*this);
    }
}

u8 ClientStatePoll::getCharacterIdsElement(u32 i0) {
    return m_writeInfo.ready->karts[m_kartIndex].characterIDs[i0];
}

u8 ClientStatePoll::getKartId() {
    return m_writeInfo.ready->karts[m_kartIndex].kartID;
}

void ClientStatePoll::getUnspecified() {}

u8 ClientStatePoll::getSpecified() {
    return *m_writeInfo.ready->courseIndex;
}
