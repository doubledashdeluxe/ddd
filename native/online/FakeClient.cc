#include "FakeClient.hh"

#include <portable/online/ClientStateIdle.hh>

FakeClient::FakeClient(const AEntries &aEntries, const SRVEntries &srvEntries, const Key &serverK,
        const std::vector<u8> &data, const Ring<ServerManager::Server, MaxServerCount> &servers,
        const Key &clientK)
    : m_dnsSocket(aEntries, srvEntries)
    , m_dns(m_dnsSocket)
    , m_serverPlatform(m_allocator, m_random, serverK, data)
    , m_socket(m_serverPlatform)
    , m_serverManager(servers)
    , m_platform(m_allocator, m_random, m_network, m_dns, m_socket, m_serverManager, clientK)
    , m_state(new (m_platform.allocator) ClientStateIdle(m_platform)) {}

FakeClient::~FakeClient() = default;

void FakeClient::read() {
    while (updateState(m_state->read(*this))) {}
}

bool FakeClient::write() {
    if (!m_writer) {
        return false;
    }

    while (updateState((this->*m_writer)())) {}
    return true;
}

bool FakeClient::clientStateIdle() {
    m_writer = &FakeClient::writeStateServer;
    return true;
}

bool FakeClient::clientStateServer(const ClientStateServerReadInfo &readInfo) {
    const ClientStateServerReadInfo::Server &server = readInfo.servers[0];
    if (server.motd) {
        m_writer = &FakeClient::writeStateMode;
    } else if (server.updateIsAvailable) {
        m_writer = &FakeClient::writeStateUpdate;
    }
    return true;
}

bool FakeClient::clientStateUpdate(const ClientStateUpdateReadInfo & /* readInfo */) {
    return true;
}

bool FakeClient::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    m_writer = &FakeClient::writeStatePack;
    return true;
}

bool FakeClient::clientStatePack(const ClientStatePackReadInfo & /* readInfo */) {
    m_writer = &FakeClient::writeStateRoom;
    return true;
}

bool FakeClient::clientStateRoom(const ClientStateRoomReadInfo & /* readInfo */) {
    m_writer = &FakeClient::writeStateTeam;
    return true;
}

bool FakeClient::clientStateTeam(const ClientStateTeamReadInfo & /* readInfo */) {
    m_writer = &FakeClient::writeStatePoll;
    return true;
}

bool FakeClient::clientStatePoll(const ClientStatePollReadInfo & /* readInfo */) {
    m_writer = &FakeClient::writeStateRace;
    return true;
}

bool FakeClient::clientStateRace(const ClientStateRaceReadInfo & /* readInfo */) {
    return true;
}

void FakeClient::clientStateError() {
    m_writer = nullptr;
}

ClientState &FakeClient::writeStateServer() {
    ClientStateServerWriteInfo writeInfo;
    writeInfo.frameRate = 0;
    writeInfo.playerCount = 1;
    writeInfo.players[0].profile = 0;
    writeInfo.players[0].name = "AAA";
    writeInfo.kartCount = 1;
    return m_state->writeStateServer(writeInfo);
}

ClientState &FakeClient::writeStateUpdate() {
    ClientStateUpdateWriteInfo writeInfo;
    writeInfo.serverIndex = 0;
    writeInfo.info.region = 'P';
    writeInfo.info.platform[0] = '\0';
    writeInfo.info.language = 0;
    return m_state->writeStateUpdate(writeInfo);
}

ClientState &FakeClient::writeStateMode() {
    ClientStateModeWriteInfo writeInfo;
    writeInfo.playerCount = 1;
    writeInfo.serverIndex = 0;
    return m_state->writeStateMode(writeInfo);
}

ClientState &FakeClient::writeStatePack() {
    ClientStatePackWriteInfo writeInfo;
    writeInfo.modeIndex = 0;
    writeInfo.packCount = 1;
    writeInfo.packs[0].courseCount = 1;
    writeInfo.packs[0].hash.fill(0xff);
    return m_state->writeStatePack(writeInfo);
}

ClientState &FakeClient::writeStateRoom() {
    ClientStateRoomWriteInfo writeInfo;
    writeInfo.isSearch = false;
    writeInfo.modeIndex = 0;
    writeInfo.isRace = true;
    writeInfo.packCourseCount = 1;
    writeInfo.packHash.fill(0xff);
    writeInfo.isHost = true;
    writeInfo.roomCounter = 0;
    writeInfo.spectatingCounter = 0;
    writeInfo.spectating = false;
    writeInfo.options.codeType = 0;
    writeInfo.options.format = 0;
    writeInfo.options.engineSize = 2;
    writeInfo.options.itemMode = 0;
    writeInfo.options.lapCount = 0;
    writeInfo.options.matchCount = 4;
    writeInfo.options.courseSelection = 0;
    writeInfo.entryIndex = 0;
    writeInfo.continuing = true;
    return m_state->writeStateRoom(writeInfo);
}

ClientState &FakeClient::writeStateTeam() {
    ClientStateTeamWriteInfo writeInfo;
    writeInfo.isHost = true;
    writeInfo.kartCount = 2;
    writeInfo.kartTeams[0] = 0;
    writeInfo.kartTeams[1] = 1;
    writeInfo.entryIndex = 0;
    writeInfo.teamCount = 2;
    writeInfo.continuing = true;
    return m_state->writeStateTeam(writeInfo);
}

ClientState &FakeClient::writeStatePoll() {
    ClientStatePollWriteInfo writeInfo;
    writeInfo.packCourseCount = 1;
    writeInfo.kartCount = 2;
    auto &ready = writeInfo.ready.emplace();
    ready.kartCount = 1;
    ready.karts[0].characterIDs[0] = CharacterID::Mario;
    ready.karts[0].characterIDs[1] = CharacterID::Luigi;
    ready.karts[0].kartID = KartID::Mario;
    ready.courseIndex = 0;
    return m_state->writeStatePoll(writeInfo);
}

ClientState &FakeClient::writeStateRace() {
    ClientStateRaceWriteInfo writeInfo;
    writeInfo.frame = 0;
    writeInfo.kartCount = 1;
    writeInfo.karts[0].inputCount = 1;
    writeInfo.karts[0].driver = 0;
    writeInfo.karts[0].posX = 0;
    writeInfo.karts[0].posY = 0;
    writeInfo.karts[0].posZ = 0;
    writeInfo.karts[0].angle = 0;
    writeInfo.karts[0].velX = 0;
    writeInfo.karts[0].velZ = 0;
    writeInfo.karts[0].itemFrames[0] = MinClientFrame;
    writeInfo.karts[0].itemFrames[1] = MinClientFrame;
    writeInfo.karts[0].rank = 0;
    writeInfo.itemCounts.fill(0);
    return m_state->writeStateRace(writeInfo);
}

bool FakeClient::updateState(ClientState &nextState) {
    bool hasChanged = &nextState != m_state.get();
    if (hasChanged) {
        m_state.reset(&nextState);
    }
    return hasChanged;
}
