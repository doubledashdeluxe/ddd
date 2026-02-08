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
    m_writeInfo = writeInfo;

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

ServerRaceKartReader *ClientStateRace::kartsElementReader(u32 i0) {
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

bool ClientStateRace::isVelYValid(s16 /* velY */) {
    return true;
}

void ClientStateRace::setVelY(s16 velY) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].velY = velY;
}

bool ClientStateRace::isVelZValid(s16 /* velZ */) {
    return true;
}

void ClientStateRace::setVelZ(s16 velZ) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].velZ = velZ;
}

bool ClientStateRace::isItemFramesCountValid(u32 /* itemFramesCount */) {
    return true;
}

void ClientStateRace::setItemFramesCount(u32 /* itemFramesCount */) {}

bool ClientStateRace::isItemFramesElementValid(u32 /* i0 */, u16 itemFramesElement) {
    return itemFramesElement >= MinClientFrame;
}

void ClientStateRace::setItemFramesElement(u32 i0, u16 itemFramesElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemFrames[i0] = itemFramesElement;
}

bool ClientStateRace::isItemIdsCountValid(u32 /* itemIdsCount */) {
    return true;
}

void ClientStateRace::setItemIdsCount(u32 /* itemIdsCount */) {}

bool ClientStateRace::isItemIdsElementValid(u32 /* i0 */, u8 /* itemIdsElement */) {
    return true;
}

void ClientStateRace::setItemIdsElement(u32 i0, u8 itemIdsElement) {
    m_readInfo.info.getOrEmplace().karts[m_kartIndex].itemIDs[i0] = itemIdsElement;
}

ClientStateRaceWriter &ClientStateRace::raceWriter() {
    return *this;
}

u16 ClientStateRace::getFrame() {
    return m_writeInfo.frame;
}

u32 ClientStateRace::getKartsCount() {
    return m_writeInfo.kartCount;
}

ClientRaceKartWriter &ClientStateRace::kartsElementWriter(u32 i0) {
    m_kartIndex = i0;
    return *this;
}

u32 ClientStateRace::getItemCountsCount() {
    return m_writeInfo.itemCounts.count();
}

u8 ClientStateRace::getItemCountsElement(u32 i0) {
    return m_writeInfo.itemCounts[i0];
}

u32 ClientStateRace::getInputsCount() {
    return m_writeInfo.karts[m_kartIndex].inputs.count();
}

u32 ClientStateRace::getInputsCount(u32 i0) {
    return m_writeInfo.karts[m_kartIndex].inputs[i0].inputCount;
}

u16 ClientStateRace::getInputsElement(u32 i0, u32 i1) {
    return m_writeInfo.karts[m_kartIndex].inputs[i0].inputs[i1];
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

s16 ClientStateRace::getVelY() {
    return m_writeInfo.karts[m_kartIndex].velY;
}

s16 ClientStateRace::getVelZ() {
    return m_writeInfo.karts[m_kartIndex].velZ;
}

u32 ClientStateRace::getItemFramesCount() {
    return m_writeInfo.karts[m_kartIndex].itemFrames.count();
}

u16 ClientStateRace::getItemFramesElement(u32 i0) {
    return m_writeInfo.karts[m_kartIndex].itemFrames[i0];
}

u8 ClientStateRace::getRank() {
    return m_writeInfo.karts[m_kartIndex].rank;
}
