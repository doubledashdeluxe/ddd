#include "ClientStateRoom.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStatePack.hh"

ClientStateRoom::ClientStateRoom(const ClientPlatform &platform, Connection &connection,
        const ClientStateRoomWriteInfo &writeInfo)
    : ClientState(platform), m_connection(&connection), m_writeInfo(writeInfo) {
    m_readInfo.ok = true;
}

ClientStateRoom::~ClientStateRoom() {}

bool ClientStateRoom::needsSockets() {
    return true;
}

ClientState &ClientStateRoom::read(ClientReadHandler &handler) {
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

    if (!handler.clientStateRoom(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateRoom::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connection.release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ClientState &ClientStateRoom::writeStatePack(const ClientStatePackWriteInfo &writeInfo) {
    Connection &connection = *m_connection.release();
    return *(new (m_platform.allocator) ClientStatePack(m_platform, connection, writeInfo));
}

ClientState &ClientStateRoom::writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) {
    checkSocket();

    m_writeInfo.spectatingCounter = writeInfo.spectatingCounter;
    m_writeInfo.spectating = writeInfo.spectating;
    m_writeInfo.options = writeInfo.options;
    m_writeInfo.entryIndex = writeInfo.entryIndex;

    Array<u8, 512> buffer;
    u32 size = buffer.count();
    Address address;
    if (m_connection->write(*this, buffer.values(), size, address)) {
        m_platform.socket.sendTo(buffer.values(), size, address);
    }

    return *this;
}

ServerStateServerReader *ClientStateRoom::serverReader() {
    return nullptr;
}

ServerStateModeReader *ClientStateRoom::modeReader() {
    return nullptr;
}

ServerStatePackReader *ClientStateRoom::packReader() {
    return nullptr;
}

ServerStateRoomReader *ClientStateRoom::roomReader() {
    return this;
}

ServerRoomStateReader *ClientStateRoom::serverRoomStateReader() {
    return this;
}

ServerRoomStateMainReader *ClientStateRoom::mainReader() {
    return this;
}

bool ClientStateRoom::isErrorValid() {
    return true;
}

void ClientStateRoom::setError() {
    m_readInfo.ok = false;
}

bool ClientStateRoom::isKartsCountValid(u32 /* kartsCount */) {
    return true;
}

void ClientStateRoom::setKartsCount(u32 kartsCount) {
    m_readInfo.info.getOrEmplace().kartCount = kartsCount;
}

ServerKartReader *ClientStateRoom::kartsElementReader(u32 i0) {
    m_kartIndex = i0;
    return this;
}

bool ClientStateRoom::isSpectatorCountValid(u16 /* spectatorCount */) {
    return true;
}

void ClientStateRoom::setSpectatorCount(u16 spectatorCount) {
    m_readInfo.info.getOrEmplace().spectatorCount = spectatorCount;
}

bool ClientStateRoom::isModeIndexValid(u8 modeIndex) {
    if (m_writeInfo.isHost) {
        return modeIndex == m_writeInfo.modeIndex;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || modeIndex == info->modeIndex;
    }
}

void ClientStateRoom::setModeIndex(u8 modeIndex) {
    m_readInfo.info.getOrEmplace().modeIndex = modeIndex;
}

bool ClientStateRoom::isPackHashCountValid(u32 /* packHashCount */) {
    return true;
}

void ClientStateRoom::setPackHashCount(u32 /* packHashCount */) {}

bool ClientStateRoom::isPackHashElementValid(u32 i0, u8 packHashElement) {
    if (m_writeInfo.isHost) {
        return packHashElement == m_writeInfo.packHash[i0];
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || packHashElement == info->packHash[i0];
    }
}

void ClientStateRoom::setPackHashElement(u32 i0, u8 packHashElement) {
    m_readInfo.info.getOrEmplace().packHash[i0] = packHashElement;
}

bool ClientStateRoom::isRoomCounterValid(u32 roomCounter) {
    return roomCounter == m_writeInfo.roomCounter;
}

void ClientStateRoom::setRoomCounter(u32 /* roomCounter */) {}

bool ClientStateRoom::isRoomCodeValid(u64 roomCode) {
    return roomCode >> (20 * 3) == 0;
}

void ClientStateRoom::setRoomCode(u64 roomCode) {
    m_readInfo.info.getOrEmplace().roomCode = roomCode;
}

bool ClientStateRoom::isSpectatingCounterValid(u32 /* spectatingCounter */) {
    return true;
}

void ClientStateRoom::setSpectatingCounter(u32 spectatingCounter) {
    m_readInfo.info.getOrEmplace().spectatingCounter = spectatingCounter;
}

bool ClientStateRoom::isSpectatingValid(u8 /* spectating */) {
    return true;
}

void ClientStateRoom::setSpectating(u8 spectating) {
    m_readInfo.info.getOrEmplace().spectating = spectating;
}

ServerRoomOptionsReader *ClientStateRoom::optionsReader() {
    return this;
}

bool ClientStateRoom::isPlayersCountValid(u32 /* playersCount */) {
    return true;
}

void ClientStateRoom::setPlayersCount(u32 playersCount) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].playerCount = playersCount;
}

ServerPlayerReader *ClientStateRoom::playersElementReader(u32 i0) {
    m_playerIndex = i0;
    return this;
}

bool ClientStateRoom::isNameCountValid(u32 /* nameCount */) {
    return true;
}

void ClientStateRoom::setNameCount(u32 nameCount) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].players[m_playerIndex].name[nameCount] = '\0';
}

bool ClientStateRoom::isNameElementValid(u32 /* i0 */, u8 nameElement) {
    return nameElement != '\0';
}

void ClientStateRoom::setNameElement(u32 i0, u8 nameElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].players[m_playerIndex].name[i0] = nameElement;
}

RoomOptionsRaceReader *ClientStateRoom::raceReader() {
    return this;
}

RoomOptionsBattleReader *ClientStateRoom::battleReader() {
    return this;
}

bool ClientStateRoom::isRaceValid() {
    if (m_writeInfo.isHost) {
        return m_writeInfo.isRace;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || info->isRace;
    }
}

void ClientStateRoom::setRace() {
    m_readInfo.info.getOrEmplace().isRace = true;
}

bool ClientStateRoom::isBattleValid() {
    if (m_writeInfo.isHost) {
        return !m_writeInfo.isRace;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || !info->isRace;
    }
}

void ClientStateRoom::setBattle() {
    m_readInfo.info.getOrEmplace().isRace = false;
}

bool ClientStateRoom::isCodeTypeValid(u8 /* codeType */) {
    return true;
}

void ClientStateRoom::setCodeType(u8 codeType) {
    m_readInfo.info.getOrEmplace().options.codeType = codeType;
}

bool ClientStateRoom::isFormatValid(u8 /* format */) {
    return true;
}

void ClientStateRoom::setFormat(u8 format) {
    m_readInfo.info.getOrEmplace().options.format = format;
}

bool ClientStateRoom::isEngineSizeValid(u8 /* engineSize */) {
    return true;
}

void ClientStateRoom::setEngineSize(u8 engineSize) {
    m_readInfo.info.getOrEmplace().options.engineSize = engineSize;
}

bool ClientStateRoom::isItemModeValid(u8 /* itemMode */) {
    return true;
}

void ClientStateRoom::setItemMode(u8 itemMode) {
    m_readInfo.info.getOrEmplace().options.itemMode = itemMode;
}

bool ClientStateRoom::isLapCountValid(u8 lapCount) {
    return lapCount <= MaxLapCount;
}

void ClientStateRoom::setLapCount(u8 lapCount) {
    m_readInfo.info.getOrEmplace().options.lapCount = lapCount;
}

bool ClientStateRoom::isMatchCountValid(u8 matchCount) {
    return matchCount >= MinMatchCount && matchCount <= MaxMatchCount;
}

void ClientStateRoom::setMatchCount(u8 matchCount) {
    m_readInfo.info.getOrEmplace().options.matchCount = matchCount;
}

bool ClientStateRoom::isCourseSelectionValid(u8 /* courseSelection */) {
    return true;
}

void ClientStateRoom::setCourseSelection(u8 courseSelection) {
    m_readInfo.info.getOrEmplace().options.courseSelection = courseSelection;
}

bool ClientStateRoom::isEntryIndexValid(u8 /* entryIndex */) {
    return true;
}

void ClientStateRoom::setEntryIndex(u8 entryIndex) {
    m_readInfo.info.getOrEmplace().entryIndex = entryIndex;
}

ClientStateRoomWriter &ClientStateRoom::roomWriter() {
    return *this;
}

ClientRoomStateWriter &ClientStateRoom::clientRoomStateWriter() {
    if (m_readInfo.info) {
        return Upcast<ClientRoomStateWriter::Main>(*this);
    } else {
        if (m_writeInfo.isHost) {
            return Upcast<ClientRoomStateWriter::New>(*this);
        } else {
            return Upcast<ClientRoomStateWriter::Code>(*this);
        }
    }
}

ClientRoomStateNewWriter &ClientStateRoom::newWriter() {
    return *this;
}

ClientRoomStateCodeWriter &ClientStateRoom::codeWriter() {
    return *this;
}

ClientRoomStateMainWriter &ClientStateRoom::mainWriter() {
    return *this;
}

u8 ClientStateRoom::getModeIndex() {
    return m_writeInfo.modeIndex;
}

u32 ClientStateRoom::getPackHashCount() {
    return m_writeInfo.packHash.count();
}

u8 ClientStateRoom::getPackHashElement(u32 i0) {
    return m_writeInfo.packHash[i0];
}

u32 ClientStateRoom::getRoomCounter() {
    return m_writeInfo.roomCounter;
}

u64 ClientStateRoom::getRoomCode() {
    return m_writeInfo.roomCode;
}

u32 ClientStateRoom::getSpectatingCounter() {
    return m_writeInfo.spectatingCounter;
}

u8 ClientStateRoom::getSpectating() {
    return m_writeInfo.spectating;
}

ClientRoomOptionsWriter &ClientStateRoom::optionsWriter() {
    if (m_writeInfo.isHost) {
        if (m_writeInfo.isRace) {
            return Upcast<ClientRoomOptionsWriter::Race>(*this);
        } else {
            return Upcast<ClientRoomOptionsWriter::Battle>(*this);
        }
    } else {
        return Upcast<ClientRoomOptionsWriter::None>(*this);
    }
}

RoomOptionsRaceWriter &ClientStateRoom::raceWriter() {
    return *this;
}

RoomOptionsBattleWriter &ClientStateRoom::battleWriter() {
    return *this;
}

u8 ClientStateRoom::getCodeType() {
    return m_writeInfo.options.codeType;
}

u8 ClientStateRoom::getFormat() {
    return m_writeInfo.options.format;
}

u8 ClientStateRoom::getEngineSize() {
    return m_writeInfo.options.engineSize;
}

u8 ClientStateRoom::getItemMode() {
    return m_writeInfo.options.itemMode;
}

u8 ClientStateRoom::getLapCount() {
    return m_writeInfo.options.lapCount;
}

u8 ClientStateRoom::getMatchCount() {
    return m_writeInfo.options.matchCount;
}

u8 ClientStateRoom::getCourseSelection() {
    return m_writeInfo.options.courseSelection;
}

u8 ClientStateRoom::getEntryIndex() {
    return m_writeInfo.entryIndex;
}

void ClientStateRoom::checkSocket() {
    if (!m_platform.socket.ok()) {
        m_connection->reset();
        m_platform.socket.open();
    }
}
