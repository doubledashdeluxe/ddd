#include "ClientStateRace.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStatePoll.hh"
#include "portable/online/ClientStateRoom.hh"

ClientStateRace::ClientStateRace(const ClientPlatform &platform, ClientState &state,
        const ClientStateRaceWriteInfo &writeInfo)
    : ClientState(platform, &state)
    , m_writeInfo(writeInfo)
    , m_serverFrame(0)
    , m_clientFrameBase(MinClientFrame)
    , m_clientFrameOffset(0) {
    m_readInfo.ok = true;
    m_readInfo.ready = true;
    m_readInfo.resultCount = 0;
}

ClientStateRace::~ClientStateRace() {}

bool ClientStateRace::needsSockets() {
    return true;
}

ClientState &ClientStateRace::read(ClientReadHandler &handler) {
    if (m_platform.replay) {
        return readReplay(handler);
    }

    ClientState::read(*this);

    if (!handler.clientStateRace(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateRace::write(const ClientStateRoomWriteInfo &writeInfo) {
    return *(new (m_platform.allocator) ClientStateRoom(m_platform, *this, writeInfo));
}

ClientState &ClientStateRace::write(const ClientStatePollWriteInfo &writeInfo) {
    return *(new (m_platform.allocator) ClientStatePoll(m_platform, *this, writeInfo));
}

ClientState &ClientStateRace::write(const ClientStateRaceWriteInfo &writeInfo) {
    m_writeInfo = writeInfo;

    ClientState::write(*this);

    return *this;
}

ServerStateServerReader<void> *ClientStateRace::serverReader() {
    return nullptr;
}

ServerStateUpdateReader<void> *ClientStateRace::updateReader() {
    return nullptr;
}

ServerStateModeReader<void> *ClientStateRace::modeReader() {
    return nullptr;
}

ServerStatePackReader<void> *ClientStateRace::packReader() {
    return nullptr;
}

ServerStateRoomReader<void> *ClientStateRace::roomReader() {
    return nullptr;
}

ServerStateTeamReader<void> *ClientStateRace::teamReader() {
    return nullptr;
}

ServerStatePollReader<void> *ClientStateRace::pollReader() {
    return nullptr;
}

ServerStateRaceReader<ClientStateRace> *ClientStateRace::raceReader() {
    return this;
}

ServerRaceStateReader<ClientStateRace> *ClientStateRace::serverRaceStateReader() {
    return this;
}

ServerRaceStateMainReader<ClientStateRace> *ClientStateRace::mainReader() {
    return this;
}

bool ClientStateRace::isErrorValid() {
    return true;
}

void ClientStateRace::setError() {
    m_readInfo.ok = false;
}

bool ClientStateRace::isMatchIndexValid(u8 /* matchIndex */) {
    return true;
}

void ClientStateRace::setMatchIndex(u8 matchIndex) {
    if (matchIndex != m_writeInfo.matchIndex) {
        m_readInfo.ok = false;
    }
}

bool ClientStateRace::isFrameValid(u16 frame) {
    return isFrameValid(m_platform.replay.get(), frame);
}

void ClientStateRace::setFrame(u16 frame) {
    m_readInfo.info.getOrEmplace().frame = frame;
}

bool ClientStateRace::isClientFrameValid(u16 clientFrame) {
    return isClientFrameValid(m_platform.replay.get(), clientFrame);
}

void ClientStateRace::setClientFrame(u16 clientFrame) {
    m_readInfo.info.getOrEmplace().clientFrame = clientFrame;
}

bool ClientStateRace::isKartFlagsValid(u8 /* kartFlags */) {
    return true;
}

void ClientStateRace::setKartFlags(u8 kartFlags) {
    m_readInfo.info.getOrEmplace().kartFlags = kartFlags;
}

bool ClientStateRace::isKartsCountValid(u32 /* kartsCount */) {
    return true;
}

void ClientStateRace::setKartsCount(u32 kartsCount) {
    m_readInfo.info.getOrEmplace().kartCount = kartsCount;
}

ServerRaceKartReader<ClientStateRace> *ClientStateRace::kartsElementReader(u32 i0) {
    m_kartIndex = i0;
    return this;
}

bool ClientStateRace::isEndFrameValid(u16 endFrame) {
    const Optional<ReadInfo::Info> &info = m_readInfo.info;
    return !info || endFrame <= info->endFrame;
}

void ClientStateRace::setEndFrame(u16 endFrame) {
    m_readInfo.info.getOrEmplace().endFrame = endFrame;
}

bool ClientStateRace::isResultsCountValid(u32 resultsCount) {
    return !m_readInfo.resultCount || resultsCount == m_readInfo.resultCount;
}

void ClientStateRace::setResultsCount(u32 resultsCount) {
    m_readInfo.resultCount = resultsCount;
}

ServerResultReader<ClientStateRace> *ClientStateRace::resultsElementReader(u32 i0) {
    m_resultIndex = i0;
    return this;
}

bool ClientStateRace::isKartFrameValid(u16 /* kartFrame */) {
    return true;
}

void ClientStateRace::setKartFrame(u16 kartFrame) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].frame = kartFrame;
}

bool ClientStateRace::isInputsCountValid(u32 /* inputsCount */) {
    return true;
}

void ClientStateRace::setInputsCount(u32 inputsCount) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].inputCount = inputsCount;
}

bool ClientStateRace::isInputsElementValid(u32 /* i0 */, u16 /* inputsElement*/) {
    return true;
}

void ClientStateRace::setInputsElement(u32 i0, u16 inputsElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].inputs[i0] = inputsElement;
}

bool ClientStateRace::isDriverValid(u8 /* driver */) {
    return true;
}

void ClientStateRace::setDriver(u8 driver) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].driver = driver;
}

bool ClientStateRace::isPosXValid(s16 /* posX */) {
    return true;
}

void ClientStateRace::setPosX(s16 posX) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].posX = posX;
}

bool ClientStateRace::isPosYValid(s16 /* posY */) {
    return true;
}

void ClientStateRace::setPosY(s16 posY) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].posY = posY;
}

bool ClientStateRace::isPosZValid(s16 /* posZ */) {
    return true;
}

void ClientStateRace::setPosZ(s16 posZ) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].posZ = posZ;
}

bool ClientStateRace::isAngleValid(s8 /* angle */) {
    return true;
}

void ClientStateRace::setAngle(s8 angle) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].angle = angle;
}

bool ClientStateRace::isVelXValid(s16 /* velX */) {
    return true;
}

void ClientStateRace::setVelX(s16 velX) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].velX = velX;
}

bool ClientStateRace::isVelZValid(s16 /* velZ */) {
    return true;
}

void ClientStateRace::setVelZ(s16 velZ) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].velZ = velZ;
}

bool ClientStateRace::isItemFramesElementValid(u32 /* i0 */, u16 itemFramesElement) {
    return itemFramesElement >= MinClientFrame;
}

void ClientStateRace::setItemFramesElement(u32 i0, u16 itemFramesElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemFrames[i0] = itemFramesElement;
}

bool ClientStateRace::isItemIdsElementValid(u32 /* i0 */, u8 /* itemIdsElement */) {
    return true;
}

void ClientStateRace::setItemIdsElement(u32 i0, u8 itemIdsElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemIDs[i0] = itemIdsElement;
}

bool ClientStateRace::isItemEventCounterValid(u8 /* itemEventCounter */) {
    return true;
}

void ClientStateRace::setItemEventCounter(u8 itemEventCounter) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEventCounter = itemEventCounter;
}

bool ClientStateRace::isItemEventsCountValid(u32 /* itemEventsCount */) {
    return true;
}

void ClientStateRace::setItemEventsCount(u32 itemEventsCount) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEventCount = itemEventsCount;
}

ItemEventReader<ClientStateRace> *ClientStateRace::itemEventsElementReader(u32 i0) {
    m_itemEventIndex = i0;
    return this;
}

bool ClientStateRace::isLapValid(u8 lap) {
    return lap <= MaxLapCount;
}

void ClientStateRace::setLap(u8 lap) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].lap = lap;
}

bool ClientStateRace::isTimeValid(u32 time) {
    return time <= MaxTime;
}

void ClientStateRace::setTime(u32 time) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].time = time;
}

bool ClientStateRace::isEventFrameValid(u8 eventFrame) {
    return eventFrame < MaxKartInputCount;
}

void ClientStateRace::setEventFrame(u8 eventFrame) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEvents[m_itemEventIndex].frame =
            eventFrame;
}

bool ClientStateRace::isEventStickYValid(s8 eventStickY) {
    return eventStickY >= MinStickY && eventStickY <= MaxStickY;
}

void ClientStateRace::setEventStickY(s8 eventStickY) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEvents[m_itemEventIndex].stickY =
            eventStickY;
}

bool ClientStateRace::isEventItemIdValid(u8 /* eventItemId */) {
    return true;
}

void ClientStateRace::setEventItemId(u8 eventItemId) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEvents[m_itemEventIndex].itemID =
            eventItemId;
}

bool ClientStateRace::isEventPosXValid(s16 /* eventPosX */) {
    return true;
}

void ClientStateRace::setEventPosX(s16 eventPosX) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEvents[m_itemEventIndex].posX = eventPosX;
}

bool ClientStateRace::isEventPosZValid(s16 /* eventPosZ */) {
    return true;
}

void ClientStateRace::setEventPosZ(s16 eventPosZ) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemEvents[m_itemEventIndex].posZ = eventPosZ;
}

bool ClientStateRace::isKartIndexValid(u8 kartIndex) {
    return !m_readInfo.resultCount || kartIndex == m_readInfo.results[m_resultIndex].kartIndex;
}

void ClientStateRace::setKartIndex(u8 kartIndex) {
    m_readInfo.results[m_resultIndex].kartIndex = kartIndex;
}

bool ClientStateRace::isResultTimeValid(u32 /* resultTime */) {
    return true;
}

void ClientStateRace::setResultTime(u32 /* resultTime */) {}

bool ClientStateRace::isPointsValid(u16 points) {
    return !m_readInfo.resultCount || points == m_readInfo.results[m_resultIndex].points;
}

void ClientStateRace::setPoints(u16 points) {
    m_readInfo.results[m_resultIndex].points = points;
}

ClientStateRaceWriter<ClientStateRace> &ClientStateRace::raceWriter() {
    return *this;
}

u16 ClientStateRace::getFrame() {
    return m_writeInfo.frame;
}

u32 ClientStateRace::getFramesCount() {
    return m_writeInfo.frames.count();
}

ClientRaceFramesWriter<ClientStateRace> &ClientStateRace::framesElementWriter(u32 i0) {
    m_frameIndex = i0;
    return *this;
}

u32 ClientStateRace::getKartsCount() {
    return m_writeInfo.kartCount;
}

ClientRaceKartWriter<ClientStateRace> &ClientStateRace::kartsElementWriter(u32 i0) {
    m_kartIndex = i0;
    return *this;
}

u8 ClientStateRace::getItemCountsElement(u32 i0) {
    return m_writeInfo.itemCounts[i0];
}

u32 ClientStateRace::getDelayedFrames() {
    return m_writeInfo.delayedFrames;
}

u16 ClientStateRace::getLatency() {
    return m_writeInfo.latency;
}

u8 ClientStateRace::getStability() {
    return m_writeInfo.stability;
}

u16 ClientStateRace::getServerFrame() {
    return m_writeInfo.frames[m_frameIndex].serverFrame;
}

u16 ClientStateRace::getClientFrame() {
    return m_writeInfo.frames[m_frameIndex].clientFrame;
}

u32 ClientStateRace::getInputsCount() {
    return m_writeInfo.karts[m_kartIndex].inputCount;
}

u32 ClientStateRace::getInputsCount(u32 /* i0 */) {
    return m_writeInfo.karts[m_kartIndex].inputs.count();
}

u16 ClientStateRace::getInputsElement(u32 i0, u32 i1) {
    return m_writeInfo.karts[m_kartIndex].inputs[i1][i0];
}

u8 ClientStateRace::getDriver() {
    return m_writeInfo.karts[m_kartIndex].driver;
}

s16 ClientStateRace::getPosX() {
    return m_writeInfo.karts[m_kartIndex].posX;
}

s16 ClientStateRace::getPosY() {
    return m_writeInfo.karts[m_kartIndex].posY;
}

s16 ClientStateRace::getPosZ() {
    return m_writeInfo.karts[m_kartIndex].posZ;
}

s8 ClientStateRace::getAngle() {
    return m_writeInfo.karts[m_kartIndex].angle;
}

s16 ClientStateRace::getVelX() {
    return m_writeInfo.karts[m_kartIndex].velX;
}

s16 ClientStateRace::getVelZ() {
    return m_writeInfo.karts[m_kartIndex].velZ;
}

u16 ClientStateRace::getItemFramesElement(u32 i0) {
    return m_writeInfo.karts[m_kartIndex].itemFrames[i0];
}

u8 ClientStateRace::getItemEventCounter() {
    return m_writeInfo.karts[m_kartIndex].itemEventCounter;
}

u32 ClientStateRace::getItemEventsCount() {
    return m_writeInfo.karts[m_kartIndex].itemEvents.count();
}

ItemEventWriter<ClientStateRace> &ClientStateRace::itemEventsElementWriter(u32 i0) {
    m_itemEventIndex = i0;
    return *this;
}

u8 ClientStateRace::getRank() {
    return m_writeInfo.karts[m_kartIndex].rank;
}

u8 ClientStateRace::getLap() {
    return m_writeInfo.karts[m_kartIndex].lap;
}

u32 ClientStateRace::getTime() {
    return m_writeInfo.karts[m_kartIndex].time;
}

u8 ClientStateRace::getEventFrame() {
    return m_writeInfo.karts[m_kartIndex].itemEvents[m_itemEventIndex].frame;
}

s8 ClientStateRace::getEventStickY() {
    return m_writeInfo.karts[m_kartIndex].itemEvents[m_itemEventIndex].stickY;
}

u8 ClientStateRace::getEventItemId() {
    return m_writeInfo.karts[m_kartIndex].itemEvents[m_itemEventIndex].itemID;
}

s16 ClientStateRace::getEventPosX() {
    return m_writeInfo.karts[m_kartIndex].itemEvents[m_itemEventIndex].posX;
}

s16 ClientStateRace::getEventPosZ() {
    return m_writeInfo.karts[m_kartIndex].itemEvents[m_itemEventIndex].posZ;
}

bool ClientStateRace::isClientStatesCountValid(u32 clientStatesCount) {
    return clientStatesCount == m_platform.replay->replay().clients.count();
}

void ClientStateRace::setClientStatesCount(u32 /* clientStatesCount */) {}

bool ClientStateRace::isClientStatesCountValid(u32 /* i0 */, u32 /* clientStatesCount */) {
    return true;
}

void ClientStateRace::setClientStatesCount(u32 i0, u32 clientStatesCount) {
    if (i0 == m_platform.replay->clientIndex()) {
        m_clientStates.reset();
        for (u32 i = 0; i < clientStatesCount; i++) {
            m_clientStates.emplaceBack();
        }
    }
}

ReplayClientStateReader<ClientStateRace> *ClientStateRace::clientStatesElementReader(u32 i0,
        u32 i1) {
    m_clientIndex = i0;
    m_stateIndex = i1;
    return this;
}

bool ClientStateRace::isReplayServerFrameValid(u16 /* replayServerFrame */) {
    return true;
}

void ClientStateRace::setReplayServerFrame(u16 replayServerFrame) {
    if (m_clientIndex == m_platform.replay->clientIndex()) {
        m_clientStates[m_stateIndex].serverFrame = replayServerFrame;
    }
}

bool ClientStateRace::isReplayClientFrameValid(u16 /* replayClientFrame */) {
    return true;
}

void ClientStateRace::setReplayClientFrame(u16 replayClientFrame) {
    if (m_clientIndex == m_platform.replay->clientIndex()) {
        m_clientStates[m_stateIndex].clientFrame = replayClientFrame;
    }
}

bool ClientStateRace::isReplayInputsCountValid(u32 replayInputsCount) {
    return replayInputsCount == m_platform.replay->replay().clients[m_clientIndex].players.count();
}

void ClientStateRace::setReplayInputsCount(u32 /* replayInputsCount */) {}

bool ClientStateRace::isReplayInputsElementValid(u32 /* i0 */, u16 /* replayInputsElement */) {
    return true;
}

void ClientStateRace::setReplayInputsElement(u32 i0, u16 replayInputsElement) {
    if (m_clientIndex == m_platform.replay->clientIndex()) {
        m_clientStates[m_stateIndex].inputs[i0] = replayInputsElement;
    }
}

ClientState &ClientStateRace::readReplay(ClientReadHandler &handler) {
    u8 buffer[4 * 1024];

    while (true) {
        if (!m_platform.replay->ok()) {
            return *(new (m_platform.allocator) ClientStateError(m_platform));
        }

        u32 size = Count(buffer);
        if (!m_platform.replay->read(buffer, size)) {
            m_readInfo.ready = false;

            if (!handler.clientStateRace(m_readInfo)) {
                return *(new (m_platform.allocator) ClientStateError(m_platform));
            }

            return *this;
        }

        if (size == 0) {
            u32 clientFrame = m_writeInfo.frame;
            if (!isClientFrameValid(false, clientFrame)) {
                break;
            }

            if (m_readInfo.info) {
                m_readInfo.info->clientFrame = clientFrame;
            }

            break;
        }

        u32 offset = 0;
        if (!ServerStateReader::isValid(buffer, size, offset)) {
            return *(new (m_platform.allocator) ClientStateError(m_platform));
        }
        m_replayOffset = offset;

        if (!ReplayStateReader::isValid(buffer, size, offset)) {
            return *(new (m_platform.allocator) ClientStateError(m_platform));
        }

        if (m_platform.replay->client() && offset < size) {
            offset = m_replayOffset;
            ReplayStateReader::read(buffer, offset);
        } else {
            m_clientStates.reset();
            if (m_serverFrame >= MinClientFrame) {
                ReplayClientState *clientState = m_clientStates.emplaceBack();
                clientState->serverFrame = m_serverFrame;
                clientState->clientFrame = m_writeInfo.frame;
            }
        }

        m_replayOffset = offset;

        while (m_clientFrameBase + m_clientFrameOffset < m_writeInfo.frame &&
                m_clientFrameOffset < m_clientStates.count()) {
            m_clientFrameOffset++;
        }

        if (m_clientFrameOffset < m_clientStates.count()) {
            const ReplayClientState &clientState = m_clientStates[m_clientFrameOffset];
            m_readInfo.replayInputs = clientState.inputs;

            if (!isFrameValid(false, clientState.serverFrame) ||
                    !isClientFrameValid(false, clientState.clientFrame)) {
                break;
            }

            offset = 0;
            ServerStateReader::read(buffer, offset);

            if (m_readInfo.info) {
                m_readInfo.info->frame = clientState.serverFrame;
                m_readInfo.info->clientFrame = clientState.clientFrame;
            }

            break;
        }

        m_platform.replay->seek(m_replayOffset);
        m_serverFrame++;
        m_clientFrameBase += m_clientStates.count();
        m_clientFrameOffset = 0;
    }

    m_readInfo.ready = true;

    if (!handler.clientStateRace(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

bool ClientStateRace::isFrameValid(bool isReplay, u16 frame) {
    if (isReplay) {
        return true;
    } else {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || frame >= info->frame;
    }
}

bool ClientStateRace::isClientFrameValid(bool isReplay, u16 clientFrame) {
    if (isReplay) {
        return true;
    } else if (clientFrame <= m_writeInfo.frame) {
        const Optional<ReadInfo::Info> &info = m_readInfo.info;
        return !info || clientFrame >= info->clientFrame;
    } else {
        return false;
    }
}
