#include "ClientStatePack.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStateRoom.hh"

ClientStatePack::ClientStatePack(const ClientPlatform &platform, Connection &connection,
        const ClientStatePackWriteInfo &writeInfo)
    : ClientState(platform), m_writeInfo(writeInfo), m_packIndex(0) {
    m_connections.pushBack();
    m_connections.back()->reset(&connection);
}

ClientStatePack::~ClientStatePack() {}

bool ClientStatePack::needsSockets() {
    return true;
}

ClientState &ClientStatePack::read(ClientReadHandler &handler) {
    ClientState::read(*this);

    if (!m_writeInfo.packIndex && m_readInfo.packIndex == m_packIndex) {
        m_packIndex = (m_packIndex + 1) % m_writeInfo.packCount;
    }

    if (!handler.clientStatePack(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStatePack::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ClientState &ClientStatePack::writeStatePack(const WriteInfo &writeInfo) {
    Optional<u32> packIndex = writeInfo.packIndex;
    m_writeInfo.packIndex = packIndex;
    if (packIndex) {
        m_packIndex = *packIndex;
    }

    ClientState::write(*this);

    return *this;
}

ClientState &ClientStatePack::writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStateRoom(m_platform, connection, writeInfo));
}

ServerStateServerReader *ClientStatePack::serverReader() {
    return nullptr;
}

ServerStateModeReader *ClientStatePack::modeReader() {
    return nullptr;
}

ServerStatePackReader *ClientStatePack::packReader() {
    return this;
}

ServerStateRoomReader *ClientStatePack::roomReader() {
    return nullptr;
}

ServerStateTeamReader *ClientStatePack::teamReader() {
    return nullptr;
}

ServerStatePollReader *ClientStatePack::pollReader() {
    return nullptr;
}

ServerStateRaceReader *ClientStatePack::raceReader() {
    return nullptr;
}

bool ClientStatePack::isModeIndexValid(u8 modeIndex) {
    return modeIndex == m_writeInfo.modeIndex;
}

void ClientStatePack::setModeIndex(u8 /* modeIndex */) {}

bool ClientStatePack::isPackIndexValid(u8 packIndex) {
    return packIndex == m_packIndex;
}

void ClientStatePack::setPackIndex(u8 packIndex) {
    m_readInfo.packIndex = packIndex;
}

bool ClientStatePack::isPlayerCountValid(u16 /* playerCount */) {
    return true;
}

void ClientStatePack::setPlayerCount(u16 playerCount) {
    m_readInfo.packs[m_packIndex].emplace().playerCount = playerCount;
}

bool ClientStatePack::isFormatPlayerCountsCountValid(u32 /* formatPlayerCountsCount */) {
    return true;
}

void ClientStatePack::setFormatPlayerCountsCount(u32 /* formatPlayerCountsCount */) {}

bool ClientStatePack::isFormatPlayerCountsElementValid(u32 /* i0 */,
        u16 /* formatPlayerCountsElement */) {
    return true;
}

void ClientStatePack::setFormatPlayerCountsElement(u32 i0, u16 formatPlayerCountsElement) {
    m_readInfo.packs[m_packIndex].emplace().formatPlayerCounts[i0] = formatPlayerCountsElement;
}

ClientStatePackWriter &ClientStatePack::packWriter() {
    return *this;
}

u8 ClientStatePack::getIsDuel() {
    return m_writeInfo.isDuel;
}

u8 ClientStatePack::getModeIndex() {
    return m_writeInfo.modeIndex;
}

u8 ClientStatePack::getPackIndex() {
    return m_packIndex;
}

u8 ClientStatePack::getPackCourseCount() {
    return m_writeInfo.packs[m_packIndex].courseCount;
}

u32 ClientStatePack::getPackHashCount() {
    return m_writeInfo.packs[m_packIndex].hash.count();
}

u8 ClientStatePack::getPackHashElement(u32 i0) {
    return m_writeInfo.packs[m_packIndex].hash[i0];
}
