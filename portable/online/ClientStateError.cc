#include "ClientStateError.hh"

ClientStateError::ClientStateError(const ClientPlatform &platform) : ClientState(platform) {
    platform.socket.close();
}

ClientStateError::~ClientStateError() {}

bool ClientStateError::needsSockets() {
    return false;
}

ClientState &ClientStateError::read(ClientReadHandler &handler) {
    handler.clientStateError((ClientStateErrorReadInfo){});
    return *this;
}

ClientState &ClientStateError::write(const ClientStateIdleWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateServerWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateUpdateWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateModeWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStatePackWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateRoomWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateTeamWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStatePollWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateRaceWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::write(const ClientStateErrorWriteInfo & /* writeInfo */) {
    return *this;
}
