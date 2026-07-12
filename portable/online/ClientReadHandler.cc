#include "ClientReadHandler.hh"

bool ClientReadHandler::clientStateIdle() {
    return false;
}

bool ClientReadHandler::clientStateServer(const ClientStateServerReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStateUpdate(const ClientStateUpdateReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStatePack(const ClientStatePackReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStateRoom(const ClientStateRoomReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStateTeam(const ClientStateTeamReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStatePoll(const ClientStatePollReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStateRace(const ClientStateRaceReadInfo & /* readInfo */) {
    return false;
}
