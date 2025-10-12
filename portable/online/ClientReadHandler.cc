#include "ClientReadHandler.hh"

bool ClientReadHandler::clientStateIdle() {
    return false;
}

bool ClientReadHandler::clientStateServer(const ClientStateServerReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStateMode(const ClientStateModeReadInfo & /* readInfo */) {
    return false;
}

bool ClientReadHandler::clientStatePack(const ClientStatePackReadInfo & /* readInfo */) {
    return false;
}
