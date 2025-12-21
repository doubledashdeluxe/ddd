#include "ClientStateError.hh"

ClientStateError::ClientStateError(const ClientPlatform &platform) : ClientState(platform) {
    platform.socket.close();
}

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

ClientState &ClientStateError::writeStateServer(
        const ClientStateServerWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStateMode(const ClientStateModeWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStatePack(const ClientStatePackWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStateRoom(const ClientStateRoomWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStateTeam(const ClientStateTeamWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStatePoll(const ClientStatePollWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStateRace(const ClientStateRaceWriteInfo & /* writeInfo */) {
    return *this;
}

ClientState &ClientStateError::writeStateError() {
    return *this;
}
