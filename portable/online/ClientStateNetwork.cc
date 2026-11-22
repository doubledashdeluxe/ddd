#include "ClientStateNetwork.hh"

#include "portable/online/ClientStateError.hh"
#include "portable/online/ClientStateServer.hh"

ClientStateNetwork::ClientStateNetwork(const ClientPlatform &platform)
    : ClientState(platform, nullptr) {
    platform.socket.close();
}

ClientStateNetwork::~ClientStateNetwork() {}

bool ClientStateNetwork::needsSockets() {
    return true;
}

ClientState &ClientStateNetwork::read(ClientReadHandler &handler) {
    if (!handler.clientStateNetwork((ClientStateNetworkReadInfo){})) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *this;
}

ClientState &ClientStateNetwork::write(const ClientStateNetworkWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateNetwork::write(const ClientStateServerWriteInfo & /* writeInfo */) {
    if (m_platform.replay) {
        return *(new (m_platform.allocator) ClientStateError(m_platform));
    }

    return *(new (m_platform.allocator) ClientStateServer(m_platform));
}
