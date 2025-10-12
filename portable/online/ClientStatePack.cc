#include "ClientStatePack.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateMode.hh"

ClientStatePack::ClientStatePack(const ClientPlatform &platform, Connection &connection,
        const ClientStatePackWriteInfo &writeInfo)
    : ClientState(platform), m_connection(&connection), m_writeInfo(writeInfo), m_packIndex(0) {}

ClientStatePack::~ClientStatePack() {}

bool ClientStatePack::needsSockets() {
    return true;
}

ClientState &ClientStatePack::read(ClientReadHandler &handler) {
    checkSocket();

    for (u32 i = 0; i < 16; i++) {
        Array<u8, 512> buffer;
        Address address;
        s32 result = m_platform.socket.recvFrom(buffer.values(), buffer.count(), address);
        if (result < 0) {
            break;
        }
        if (m_connection->read(*this, buffer.values(), result, address)) {
            m_packIndex = (m_packIndex + 1) % m_writeInfo.packCount;
        }
    }

    if (!handler.clientStatePack(m_readInfo)) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStatePack::writeStateMode(const ClientStateModeWriteInfo &writeInfo) {
    Connection &connection = *m_connection.release();
    u8 playerCount = writeInfo.playerCount;
    return *(new (m_platform.allocator) ClientStateMode(m_platform, connection, playerCount));
}

ClientState &ClientStatePack::writeStatePack(const WriteInfo & /* writeInfo */) {
    checkSocket();

    Array<u8, 512> buffer;
    u32 size = buffer.count();
    Address address;
    if (m_connection->write(*this, buffer.values(), size, address)) {
        m_platform.socket.sendTo(buffer.values(), size, address);
    }

    return *this;
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

bool ClientStatePack::isModeIndexValid(u8 modeIndex) {
    return modeIndex == m_writeInfo.modeIndex;
}

void ClientStatePack::setModeIndex(u8 /* modeIndex */) {}

bool ClientStatePack::isPackIndexValid(u8 packIndex) {
    return packIndex == m_packIndex;
}

void ClientStatePack::setPackIndex(u8 /* packIndex */) {}

bool ClientStatePack::isPlayerCountValid(u16 /* playerCount */) {
    return true;
}

void ClientStatePack::setPlayerCount(u16 playerCount) {
    m_readInfo.packs[m_packIndex].emplace().playerCount = playerCount;
}

ClientStatePackWriter &ClientStatePack::packWriter() {
    return *this;
}

u8 ClientStatePack::getModeIndex() {
    return m_writeInfo.modeIndex;
}

u8 ClientStatePack::getPackIndex() {
    return m_packIndex;
}

u32 ClientStatePack::getPackHashCount() {
    return m_writeInfo.packs[m_packIndex].hash.count();
}

u8 ClientStatePack::getPackHashElement(u32 i0) {
    return m_writeInfo.packs[m_packIndex].hash[i0];
}

void ClientStatePack::checkSocket() {
    if (!m_platform.socket.ok()) {
        m_connection->reset();
        m_platform.socket.open();
    }
}
