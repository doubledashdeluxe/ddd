#include "ClientStateIdle.hh"

#include "payload/online/ClientStateError.hh"
#include "payload/online/ClientStateServer.hh"

ClientStateIdle::ClientStateIdle(ClientPlatform &platform) : ClientState(platform) {
    platform.socket.close();
}

ClientStateIdle::~ClientStateIdle() {}

bool ClientStateIdle::needsSockets() {
    return false;
}

ClientState &ClientStateIdle::read(ClientReadHandler &handler) {
    if (!handler.clientStateIdle()) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateIdle::writeStateIdle() {
    return *this;
}

ClientState &ClientStateIdle::writeStateServer() {
    return *(new (m_platform.allocator) ClientStateServer(m_platform));
}
