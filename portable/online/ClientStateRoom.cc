#include "ClientStateRoom.hh"

#include "portable/Upcast.hh"
#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"
#include "portable/online/ClientStatePack.hh"
#include "portable/online/ClientStatePoll.hh"
#include "portable/online/ClientStateTeam.hh"

ClientStateRoom::ClientStateRoom(const ClientPlatform &platform, Connection &connection,
        const ClientStateRoomWriteInfo &writeInfo)
    : ClientState(platform)
    , m_writeInfo(writeInfo) {
    m_connections.emplaceBack()->reset(&connection);
    m_readInfo.ok = true;
}

ClientStateRoom::~ClientStateRoom() {}

bool ClientStateRoom::needsSockets() {
    return true;
}

ClientState &ClientStateRoom::read(ClientReadHandler &handler) {
    ClientState::read(*this);

    Optional<ReadInfo::Info> &info = m_readInfo.info;
    if (info) {
        for (u32 i = 0; i < info->kartCount; i++) {
            Kart &kart = info->karts[i];
            for (u32 j = 0; j < kart.players.count(); j++) {
                Player &player = kart.players[j];
                if (j < kart.playerCount) {
                    player.name[PlayerNameLength] = '\0';
                } else {
                    player.index = UINT8_MAX;
                    player.name = "   ";
                }
            }
        }
    }

    if (!handler.clientStateRoom(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateRoom::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ClientState &ClientStateRoom::writeStatePack(const ClientStatePackWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStatePack(m_platform, connection, writeInfo));
}

ClientState &ClientStateRoom::writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) {
    m_writeInfo.spectatingCounter = writeInfo.spectatingCounter;
    m_writeInfo.spectating = writeInfo.spectating;
    m_writeInfo.options = writeInfo.options;
    m_writeInfo.entryIndex = writeInfo.entryIndex;
    m_writeInfo.continuing = writeInfo.continuing;

    ClientState::write(*this);

    return *this;
}

ClientState &ClientStateRoom::writeStateTeam(const ClientStateTeamWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStateTeam(m_platform, connection, writeInfo));
}

ClientState &ClientStateRoom::writeStatePoll(const ClientStatePollWriteInfo &writeInfo) {
    Connection &connection = *m_connections.front()->release();
    return *(new (m_platform.allocator) ClientStatePoll(m_platform, connection, writeInfo));
}

ServerStateServerReader<void> *ClientStateRoom::serverReader() {
    return nullptr;
}

ServerStateModeReader<void> *ClientStateRoom::modeReader() {
    return nullptr;
}

ServerStatePackReader<void> *ClientStateRoom::packReader() {
    return nullptr;
}

ServerStateRoomReader<ClientStateRoom> *ClientStateRoom::roomReader() {
    return this;
}

ServerStateTeamReader<void> *ClientStateRoom::teamReader() {
    return nullptr;
}

ServerStatePollReader<void> *ClientStateRoom::pollReader() {
    return nullptr;
}

ServerStateRaceReader<void> *ClientStateRoom::raceReader() {
    return nullptr;
}

ServerRoomStateReader<ClientStateRoom> *ClientStateRoom::serverRoomStateReader() {
    return this;
}

ServerRoomStateMainReader<ClientStateRoom> *ClientStateRoom::mainReader() {
    return this;
}

bool ClientStateRoom::isErrorValid() {
    return true;
}

void ClientStateRoom::setError() {
    m_readInfo.ok = false;
}

bool ClientStateRoom::isKartsCountValid(u32 kartsCount) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    if (info && info->continuing) {
        return kartsCount == info->kartCount;
    } else {
        return !m_writeInfo.isSearch || m_writeInfo.options.format != RoomOptionFormat::Duel ||
                kartsCount <= 2;
    }
}

void ClientStateRoom::setKartsCount(u32 kartsCount) {
    m_readInfo.info.getOrEmplace().kartCount = kartsCount;
}

ServerKartReader<ClientStateRoom> *ClientStateRoom::kartsElementReader(u32 i0) {
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
    if (m_writeInfo.isSearch || m_writeInfo.isHost) {
        return modeIndex == m_writeInfo.modeIndex;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || modeIndex == info->modeIndex;
    }
}

void ClientStateRoom::setModeIndex(u8 modeIndex) {
    m_readInfo.info.getOrEmplace().modeIndex = modeIndex;
}

bool ClientStateRoom::isPackCourseCountValid(u8 packCourseCount) {
    if (m_writeInfo.isSearch || m_writeInfo.isHost) {
        return packCourseCount == m_writeInfo.packCourseCount;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || packCourseCount == info->packCourseCount;
    }
}

void ClientStateRoom::setPackCourseCount(u8 packCourseCount) {
    m_readInfo.info.getOrEmplace().packCourseCount = packCourseCount;
}

bool ClientStateRoom::isPackHashElementValid(u32 i0, u8 packHashElement) {
    if (m_writeInfo.isSearch || m_writeInfo.isHost) {
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
    if (m_writeInfo.isSearch) {
        return roomCode == UINT64_MAX;
    } else {
        return roomCode >> (20 * 3) == 0;
    }
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

bool ClientStateRoom::isSpectatingValid(u8 spectating) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || spectating == info->spectating;
}

void ClientStateRoom::setSpectating(u8 spectating) {
    m_readInfo.info.getOrEmplace().spectating = spectating;
}

ServerRoomOptionsReader<ClientStateRoom> *ClientStateRoom::optionsReader() {
    return this;
}

bool ClientStateRoom::isContinuingValid(u8 continuing) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || continuing;
}

void ClientStateRoom::setContinuing(u8 continuing) {
    m_readInfo.info.getOrEmplace().continuing = continuing;
}

bool ClientStateRoom::isLocalValid(u8 local) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || local == info->karts[m_kartIndex].local;
}

void ClientStateRoom::setLocal(u8 local) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].local = local;
}

bool ClientStateRoom::isPlayersCountValid(u32 playersCount) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || playersCount == info->karts[m_kartIndex].playerCount;
}

void ClientStateRoom::setPlayersCount(u32 playersCount) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].playerCount = playersCount;
}

ServerPlayerReader<ClientStateRoom> *ClientStateRoom::playersElementReader(u32 i0) {
    m_playerIndex = i0;
    return this;
}

bool ClientStateRoom::isMmrValid(u16 mmr) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || mmr == info->karts[m_kartIndex].mmr;
}

void ClientStateRoom::setMmr(u16 mmr) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].mmr = mmr;
}

bool ClientStateRoom::isIndexValid(u8 index) {
    return index < MaxClientPlayerCount;
}

void ClientStateRoom::setIndex(u8 index) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].players[m_playerIndex].index = index;
}

bool ClientStateRoom::isNameElementValid(u32 /* i0 */, u8 nameElement) {
    return nameElement != '\0';
}

void ClientStateRoom::setNameElement(u32 i0, u8 nameElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].players[m_playerIndex].name[i0] = nameElement;
}

RoomOptionsRaceReader<ClientStateRoom> *ClientStateRoom::raceOptionsReader() {
    return this;
}

RoomOptionsBattleReader<ClientStateRoom> *ClientStateRoom::battleOptionsReader() {
    return this;
}

bool ClientStateRoom::isRaceValid() {
    if (m_writeInfo.isSearch || m_writeInfo.isHost) {
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
    if (m_writeInfo.isSearch || m_writeInfo.isHost) {
        return !m_writeInfo.isRace;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || !info->isRace;
    }
}

void ClientStateRoom::setBattle() {
    m_readInfo.info.getOrEmplace().isRace = false;
}

bool ClientStateRoom::isCodeTypeValid(u8 codeType) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || codeType == info->options.codeType;
}

void ClientStateRoom::setCodeType(u8 codeType) {
    m_readInfo.info.getOrEmplace().options.codeType = codeType;
}

bool ClientStateRoom::isFormatValid(u8 format) {
    if (m_writeInfo.isSearch) {
        return format == m_writeInfo.options.format;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || !info->continuing || format == info->options.format;
    }
}

void ClientStateRoom::setFormat(u8 format) {
    m_readInfo.info.getOrEmplace().options.format = format;
}

bool ClientStateRoom::isEngineSizeValid(u8 engineSize) {
    if (m_writeInfo.isSearch) {
        return engineSize == RoomOptionEngineSize::Large;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || !info->continuing || engineSize == info->options.engineSize;
    }
}

void ClientStateRoom::setEngineSize(u8 engineSize) {
    m_readInfo.info.getOrEmplace().options.engineSize = engineSize;
}

bool ClientStateRoom::isItemModeValid(u8 itemMode) {
    if (m_writeInfo.isSearch) {
        return itemMode == RoomOptionItemMode::Recommended;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || !info->continuing || itemMode == info->options.itemMode;
    }
}

void ClientStateRoom::setItemMode(u8 itemMode) {
    m_readInfo.info.getOrEmplace().options.itemMode = itemMode;
}

bool ClientStateRoom::isLapCountValid(u8 lapCount) {
    if (m_writeInfo.isSearch) {
        return lapCount == 0;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        if (info && info->continuing) {
            return lapCount == info->options.lapCount;
        } else {
            return lapCount <= MaxLapCount;
        }
    }
}

void ClientStateRoom::setLapCount(u8 lapCount) {
    m_readInfo.info.getOrEmplace().options.lapCount = lapCount;
}

bool ClientStateRoom::isMatchCountValid(u8 matchCount) {
    if (m_writeInfo.isSearch) {
        return matchCount == m_writeInfo.options.matchCount;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        if (info && info->continuing) {
            return matchCount == info->options.matchCount;
        } else {
            return matchCount >= MinMatchCount && matchCount <= MaxMatchCount;
        }
    }
}

void ClientStateRoom::setMatchCount(u8 matchCount) {
    m_readInfo.info.getOrEmplace().options.matchCount = matchCount;
}

bool ClientStateRoom::isCourseSelectionValid(u8 courseSelection) {
    if (m_writeInfo.isSearch) {
        return courseSelection == RoomOptionCourseSelection::Poll;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || !info->continuing || courseSelection == info->options.courseSelection;
    }
}

void ClientStateRoom::setCourseSelection(u8 courseSelection) {
    m_readInfo.info.getOrEmplace().options.courseSelection = courseSelection;
}

bool ClientStateRoom::isEntryIndexValid(u8 entryIndex) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || !info->continuing || entryIndex == info->entryIndex;
}

void ClientStateRoom::setEntryIndex(u8 entryIndex) {
    m_readInfo.info.getOrEmplace().entryIndex = entryIndex;
}

ClientStateRoomWriter<ClientStateRoom> &ClientStateRoom::roomWriter() {
    return *this;
}

ClientRoomStateWriter<ClientStateRoom> &ClientStateRoom::clientRoomStateWriter() {
    if (m_readInfo.info) {
        return Upcast<ClientRoomStateWriter::Main>(*this);
    } else {
        if (m_writeInfo.isSearch) {
            return Upcast<ClientRoomStateWriter::Search>(*this);
        } else if (m_writeInfo.isHost) {
            return Upcast<ClientRoomStateWriter::New>(*this);
        } else {
            return Upcast<ClientRoomStateWriter::Code>(*this);
        }
    }
}

ClientRoomStateSearchWriter<ClientStateRoom> &ClientStateRoom::searchWriter() {
    return *this;
}

ClientRoomStateNewWriter<ClientStateRoom> &ClientStateRoom::newWriter() {
    return *this;
}

ClientRoomStateCodeWriter<ClientStateRoom> &ClientStateRoom::codeWriter() {
    return *this;
}

ClientRoomStateMainWriter<ClientStateRoom> &ClientStateRoom::mainWriter() {
    return *this;
}

u8 ClientStateRoom::getModeIndex() {
    return m_writeInfo.modeIndex;
}

u8 ClientStateRoom::getPackCourseCount() {
    return m_writeInfo.packCourseCount;
}

u8 ClientStateRoom::getPackHashElement(u32 i0) {
    return m_writeInfo.packHash[i0];
}

u8 ClientStateRoom::getFormat() {
    return m_writeInfo.options.format;
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

ClientRoomOptionsWriter<ClientStateRoom> &ClientStateRoom::optionsWriter() {
    if (m_writeInfo.isHost) {
        if (m_writeInfo.isRace) {
            return Upcast<ClientRoomOptionsWriter::RaceOptions>(*this);
        } else {
            return Upcast<ClientRoomOptionsWriter::BattleOptions>(*this);
        }
    } else {
        return Upcast<ClientRoomOptionsWriter::None>(*this);
    }
}

u8 ClientStateRoom::getContinuing() {
    return m_writeInfo.continuing;
}

RoomOptionsRaceWriter<ClientStateRoom> &ClientStateRoom::raceOptionsWriter() {
    return *this;
}

RoomOptionsBattleWriter<ClientStateRoom> &ClientStateRoom::battleOptionsWriter() {
    return *this;
}

void ClientStateRoom::getNone() {}

void ClientStateRoom::getRace() {}

void ClientStateRoom::getBattle() {}

u8 ClientStateRoom::getCodeType() {
    return m_writeInfo.options.codeType;
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
