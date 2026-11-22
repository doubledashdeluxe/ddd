#include "ClientStateIdle.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateNetwork.hh"
#include "portable/online/ClientStateRoom.hh"

ClientStateIdle::ClientStateIdle(const ClientPlatform &platform) : ClientState(platform, nullptr) {
    platform.socket.close();
}

ClientStateIdle::~ClientStateIdle() {}

bool ClientStateIdle::needsSockets() {
    return false;
}

ClientState &ClientStateIdle::read(ClientReadHandler &handler) {
    if (!handler.clientStateIdle((ClientStateIdleReadInfo){})) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateIdle::write(const ClientStateIdleWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateIdle::write(const ClientStateNetworkWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateNetwork(m_platform));
}

ClientState &ClientStateIdle::write(const ClientStateRoomWriteInfo &writeInfo) {
    if (!m_platform.replay) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *(new (m_platform.allocator) ClientStateRoom(m_platform, *this, writeInfo));
}
