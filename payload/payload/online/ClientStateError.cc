#include "ClientStateError.hh"

#include "payload/online/ClientStateIdle.hh"

ClientStateError::ClientStateError(ClientPlatform &platform) : ClientState(platform) {}

ClientStateError::~ClientStateError() {}

bool ClientStateError::needsSockets() {
    return false;
}

ClientState &ClientStateError::read(ClientReadHandler &handler) {
    handler.clientStateError();
    return *this;
}

ClientState &ClientStateError::writeStateIdle() {
    return *this;
}

ClientState &ClientStateError::writeStateServer() {
    return *this;
}

ClientState &ClientStateError::writeStateRoom() {
    return *this;
}

ClientState &ClientStateError::writeStateError() {
    return *this;
}
