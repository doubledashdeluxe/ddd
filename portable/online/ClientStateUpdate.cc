#include "ClientStateUpdate.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateServer.hh"

ClientStateUpdate::ClientStateUpdate(const ClientPlatform &platform, Connection &connection,
        const Array<char, MaxVersionLength + 1> &version,
        const ClientStateUpdateWriteInfo &writeInfo)
    : ClientState(platform)
    , m_writeInfo(writeInfo) {
    m_connections.emplaceBack()->reset(&connection);
    m_readInfo.version = version;
}

ClientStateUpdate::~ClientStateUpdate() {}

bool ClientStateUpdate::needsSockets() {
    return true;
}

ClientState &ClientStateUpdate::read(ClientReadHandler &handler) {
    m_readInfo.data.chunks.reset();
    ClientState::read(*this);

    if (!handler.clientStateUpdate(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateUpdate::writeStateServer(
        const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateServer(m_platform));
}

ClientState &ClientStateUpdate::writeStateUpdate(const WriteInfo &writeInfo) {
    m_writeInfo.data = writeInfo.data;

    ClientState::write(*this);

    return *this;
}

ServerStateServerReader<void> *ClientStateUpdate::serverReader() {
    return nullptr;
}

ServerStateUpdateReader<ClientStateUpdate> *ClientStateUpdate::updateReader() {
    return this;
}

ServerStateModeReader<void> *ClientStateUpdate::modeReader() {
    return nullptr;
}

ServerStatePackReader<void> *ClientStateUpdate::packReader() {
    return nullptr;
}

ServerStateRoomReader<void> *ClientStateUpdate::roomReader() {
    return nullptr;
}

ServerStateTeamReader<void> *ClientStateUpdate::teamReader() {
    return nullptr;
}

ServerStatePollReader<void> *ClientStateUpdate::pollReader() {
    return nullptr;
}

ServerStateRaceReader<void> *ClientStateUpdate::raceReader() {
    return nullptr;
}

ServerUpdateStateReader<ClientStateUpdate> *ClientStateUpdate::serverUpdateStateReader() {
    return this;
}

ServerUpdateStateInfoReader<ClientStateUpdate> *ClientStateUpdate::infoReader() {
    if (!m_readInfo.info) {
        return this;
    }
    return nullptr;
}

ServerUpdateStateDataReader<ClientStateUpdate> *ClientStateUpdate::dataReader() {
    if (m_readInfo.info) {
        return this;
    }
    return nullptr;
}

bool ClientStateUpdate::isSizeValid(u32 size) {
    return size <= MaxUpdateSize;
}

void ClientStateUpdate::setSize(u32 size) {
    m_readInfo.info.getOrEmplace().size = size;
}

bool ClientStateUpdate::isChangelogCountValid(u32 /* changelogCount */) {
    return true;
}

void ClientStateUpdate::setChangelogCount(u32 changelogCount) {
    m_readInfo.info.getOrEmplace().changelog[changelogCount] = '\0';
}

bool ClientStateUpdate::isChangelogElementValid(u32 /* i0 */, u8 changelogElement) {
    return changelogElement != '\0';
}

void ClientStateUpdate::setChangelogElement(u32 i0, u8 changelogElement) {
    m_readInfo.info.getOrEmplace().changelog[i0] = changelogElement;
}

bool ClientStateUpdate::isIndexValid(u16 index) {
    return index * UpdateChunkSize < m_readInfo.info->size;
}

void ClientStateUpdate::setIndex(u16 index) {
    m_readInfo.data.chunks.emplaceBack();
    m_readInfo.data.chunks.back()->index = index;
}

bool ClientStateUpdate::isChunkElementValid(u32 /* i0 */, u8 /* chunkElement */) {
    return true;
}

void ClientStateUpdate::setChunkElement(u32 i0, u8 chunkElement) {
    m_readInfo.data.chunks.back()->chunk[i0] = chunkElement;
}

ClientStateUpdateWriter<ClientStateUpdate> &ClientStateUpdate::updateWriter() {
    return *this;
}

ClientUpdateStateWriter<ClientStateUpdate> &ClientStateUpdate::clientUpdateStateWriter() {
    if (m_writeInfo.data) {
        return Upcast<ClientUpdateStateWriter::Data>(*this);
    } else {
        return Upcast<ClientUpdateStateWriter::Info>(*this);
    }
}

ClientUpdateStateInfoWriter<ClientStateUpdate> &ClientStateUpdate::infoWriter() {
    return *this;
}

ClientUpdateStateDataWriter<ClientStateUpdate> &ClientStateUpdate::dataWriter() {
    return *this;
}

u8 ClientStateUpdate::getRegion() {
    return m_writeInfo.info.region;
}

u32 ClientStateUpdate::getPlatformCount() {
    return strlen(m_writeInfo.info.platform.values());
}

u8 ClientStateUpdate::getPlatformElement(u32 i0) {
    return m_writeInfo.info.platform[i0];
}

u8 ClientStateUpdate::getLanguage() {
    return m_writeInfo.info.language;
}

u32 ClientStateUpdate::getIndicesCount() {
    return m_writeInfo.data->indices.count();
}

u16 ClientStateUpdate::getIndicesElement(u32 i0) {
    return m_writeInfo.data->indices[i0];
}
