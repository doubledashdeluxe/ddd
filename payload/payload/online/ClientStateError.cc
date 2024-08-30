#include "ClientStateError.hh"

ClientStateError::ClientStateError(JKRHeap *heap) : ClientState(heap) {}

ClientStateError::~ClientStateError() {}

ClientState &ClientStateError::read(ClientReadHandler &handler) {
    handler.clientStateError();
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
