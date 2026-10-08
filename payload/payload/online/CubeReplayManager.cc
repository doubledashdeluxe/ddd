#include "CubeReplayManager.hh"

#include "payload/CourseManager.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <game/Modes.hh>
#include <portable/Algorithm.hh>
#include <portable/Log.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

void CubeReplayManager::filterAndSort() {
    CourseManager *courseManager = CourseManager::Instance();
    for (u32 i = 0; i < m_replays.count();) {
        Replay &replay = m_replays[i];
        u32 raceMode = Modes[replay.modeIndex];
        replay.isRace = RaceMode::IsRace(raceMode);
        Optional<u32> packIndex = courseManager->searchPack(true, replay.isRace,
                replay.packCourseCount, replay.packHash);
        if (packIndex &&
                replay.courseIndex < courseManager->courseCount(true, replay.isRace, *packIndex)) {
            replay.packIndex = *packIndex;
            i++;
        } else {
            m_replays.swapRemoveBack(i);
        }
    }
    Sort(m_replays, m_replays.count(), CompareReplaysByTime);
}

u32 CubeReplayManager::replayCount() const {
    return m_replays.count();
}

const CubeReplayManager::Replay &CubeReplayManager::replay(u32 index) const {
    return m_replays[index];
}

bool CubeReplayManager::isMagicValid(u32 magic) {
    return magic == ReplayMagic;
}

void CubeReplayManager::setMagic(u32 /* magic */) {}

bool CubeReplayManager::isReplayVersionValid(u16 replayVersion) {
    return replayVersion == ReplayVersion;
}

void CubeReplayManager::setReplayVersion(u16 /* replayVersion */) {}

bool CubeReplayManager::isReservedValid(u16 /* reserved */) {
    return true;
}

void CubeReplayManager::setReserved(u16 /*reserved */) {}

bool CubeReplayManager::isVersionCountValid(u32 /* versionCount */) {
    return true;
}

void CubeReplayManager::setVersionCount(u32 /* versionCount */) {}

bool CubeReplayManager::isVersionElementValid(u32 /* i0 */, u8 versionElement) {
    return versionElement != '\0';
}

void CubeReplayManager::setVersionElement(u32 /* i0 */, u8 /* versionElement */) {}

bool CubeReplayManager::isFrameRateValid(u8 /* frameRate */) {
    return true;
}

void CubeReplayManager::setFrameRate(u8 /* frameRate */) {}

bool CubeReplayManager::isModeIndexValid(u8 /* modeIndex */) {
    return true;
}

void CubeReplayManager::setModeIndex(u8 modeIndex) {
    m_replay->modeIndex = modeIndex;
}

bool CubeReplayManager::isPackCourseCountValid(u8 /* packCourseCount */) {
    return true;
}

void CubeReplayManager::setPackCourseCount(u8 packCourseCount) {
    m_replay->packCourseCount = packCourseCount;
}

bool CubeReplayManager::isPackHashElementValid(u32 /* i0 */, u8 /* packHashElement */) {
    return true;
}

void CubeReplayManager::setPackHashElement(u32 i0, u8 packHashElement) {
    m_replay->packHash[i0] = packHashElement;
}

bool CubeReplayManager::isCourseIndexValid(u8 /* courseIndex */) {
    return true;
}

void CubeReplayManager::setCourseIndex(u8 courseIndex) {
    m_replay->courseIndex = courseIndex;
}

bool CubeReplayManager::isClientsCountValid(u32 /* clientsCount */) {
    return true;
}

void CubeReplayManager::setClientsCount(u32 clientsCount) {
    m_replay->clients.reset();
    for (u32 i = 0; i < clientsCount; i++) {
        m_replay->clients.emplaceBack();
    }
}

ReplayClientReader<CubeReplayManager> *CubeReplayManager::clientsElementReader(u32 i0) {
    m_clientIndex = i0;
    return this;
}

bool CubeReplayManager::isTimeValid(u64 /* time */) {
    return true;
}

void CubeReplayManager::setTime(u64 time) {
    s64 epoch = 946684800; // 2000-01-01
    m_replay->time = Clock::SecondsToTicks(time - epoch);
}

bool CubeReplayManager::isRoomTypeValid(u8 /* roomType */) {
    return true;
}

void CubeReplayManager::setRoomType(u8 roomType) {
    m_replay->roomType = roomType;
}

bool CubeReplayManager::isFormatValid(u8 /* format */) {
    return true;
}

void CubeReplayManager::setFormat(u8 format) {
    m_replay->format = format;
}

bool CubeReplayManager::isRoomCodeValid(u64 /* roomCode */) {
    return true;
}

void CubeReplayManager::setRoomCode(u64 roomCode) {
    m_replay->roomCode = roomCode;
}

bool CubeReplayManager::isPkElementValid(u32 /* i0 */, u8 /* pkElement */) {
    return true;
}

void CubeReplayManager::setPkElement(u32 /* i0 */, u8 /* pkElement */) {}

bool CubeReplayManager::isRegionValid(u8 /* region */) {
    return true;
}

void CubeReplayManager::setRegion(u8 /* region */) {}

bool CubeReplayManager::isPlatformCountValid(u32 /* platformCount */) {
    return true;
}

void CubeReplayManager::setPlatformCount(u32 /* platformCount */) {}

bool CubeReplayManager::isPlatformElementValid(u32 /* i0 */, u8 platformElement) {
    return platformElement != '\0';
}

void CubeReplayManager::setPlatformElement(u32 /* i0 */, u8 /* platformElement */) {}

bool CubeReplayManager::isPlayersCountValid(u32 /* playersCount */) {
    return true;
}

void CubeReplayManager::setPlayersCount(u32 playersCount) {
    m_replay->clients[m_clientIndex].players.reset();
    for (u32 i = 0; i < playersCount; i++) {
        Player *player = m_replay->clients[m_clientIndex].players.emplaceBack();
        player->name[PlayerNameLength] = '\0';
    }
}

ClientPlayerReader<CubeReplayManager> *CubeReplayManager::playersElementReader(u32 i0) {
    m_playerIndex = i0;
    return this;
}

bool CubeReplayManager::isTeamsCountValid(u32 /* teamsCount */) {
    return true;
}

void CubeReplayManager::setTeamsCount(u32 teamsCount) {
    m_replay->clients[m_clientIndex].teams.reset();
    for (u32 i = 0; i < teamsCount; i++) {
        m_replay->clients[m_clientIndex].teams.emplaceBack();
    }
}

bool CubeReplayManager::isTeamsElementValid(u32 /* i0 */, u8 /* teamsElement */) {
    return true;
}

void CubeReplayManager::setTeamsElement(u32 i0, u8 teamsElement) {
    m_replay->clients[m_clientIndex].teams[i0] = teamsElement;
}

bool CubeReplayManager::isProfileValid(u8 /* profile */) {
    return true;
}

void CubeReplayManager::setProfile(u8 /* profile */) {}

bool CubeReplayManager::isNameElementValid(u32 /* i0 */, u8 nameElement) {
    return nameElement != '\0';
}

void CubeReplayManager::setNameElement(u32 i0, u8 nameElement) {
    m_replay->clients[m_clientIndex].players[m_playerIndex].name[i0] = nameElement;
}

void CubeReplayManager::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x4) CubeReplayManager;
}

CubeReplayManager *CubeReplayManager::Instance() {
    return s_instance;
}

CubeReplayManager::CubeReplayManager() {
    StorageScanner *param = this;
    OSCreateThread(&m_thread, Run, param, m_stack.values() + m_stack.count(), m_stack.count(), 27,
            0);
}

OSThread &CubeReplayManager::thread() {
    return m_thread;
}

void CubeReplayManager::process() {
    m_replays.reset();
    m_replay = m_replays.emplaceBack();
    Array<char, 128> path;
    snprintf(path.values(), path.count(), "main:/ddd/replays");
    Storage::CreateDir(path.values(), Storage::Mode::WriteAlways);
    Storage::NodeInfo nodeInfo;
    addReplays(path, nodeInfo);
    if (m_replay) {
        m_replays.popBack();
    }
}

void CubeReplayManager::addReplays(Array<char, 128> &path, Storage::NodeInfo &nodeInfo) {
    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            addReplays(path, nodeInfo);
        } else {
            addReplay(path);
        }
    }
    path[length] = '\0';
}

void CubeReplayManager::addReplay(const Array<char, 128> &path) {
    if (!m_replay) {
        return;
    }

    alignas(0x20) u8 buffer[512];
    u32 size;
    if (!Storage::ReadFile(path.values(), buffer, Count(buffer), &size)) {
        return;
    }

    u32 offset = 0;
    if (!ReplayReader::isValid(buffer, size, offset)) {
        return;
    }
    if (!ReplayRaceReader::isValid(buffer, size, offset)) {
        return;
    }
    offset = 0;
    ReplayReader::read(buffer, offset);
    ReplayRaceReader::read(buffer, offset);

    m_replay->kartCount = 0;
    for (u32 i = 0; i < m_replay->clients.count(); i++) {
        Client &client = m_replay->clients[i];
        if (client.teams.count() * 2 < client.players.count() ||
                client.teams.count() > client.players.count()) {
            return;
        }
        client.kartFlags = 0;
        for (u32 j = 0; j < client.teams.count(); j++) {
            client.kartFlags |= 1 << m_replay->kartCount;
            m_replay->kartCount++;
        }
    }
    if (m_replay->kartCount > MaxRoomKartCount) {
        return;
    }
    m_replay->path = path;
    m_replay->offset = offset;

    DEBUG("Adding replay %s...", path.values());
    m_replay = m_replays.emplaceBack();
}

bool CubeReplayManager::CompareReplaysByTime(const Replay &a, const Replay &b) {
    return a.time > b.time;
}

CubeReplayManager *CubeReplayManager::s_instance = nullptr;
