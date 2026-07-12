#include "ClientStateRace.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStatePoll.hh"

ClientStateRace::ClientStateRace(const ClientPlatform &platform, Connection &connection,
        const ClientStateRaceWriteInfo &writeInfo)
    : ClientState(platform)
    , m_writeInfo(writeInfo) {
    m_connections.emplaceBack()->reset(&connection);
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

ClientStateRaceWriter<ClientStateRace> &ClientStateRace::raceWriter() {
    return *this;
}

u16 ClientStateRace::getFrame() {
    return m_writeInfo.frame;
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
