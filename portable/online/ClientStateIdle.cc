#include "ClientStateIdle.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateServer.hh"

ClientStateIdle::ClientStateIdle(const ClientPlatform &platform) : ClientState(platform) {
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

ClientState &ClientStateIdle::writeStateServer(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *(new (m_platform.allocator) ClientStateServer(m_platform));
}
