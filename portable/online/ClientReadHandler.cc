#include "ClientReadHandler.hh"

bool ClientReadHandler::clientStateIdle() {
    return false;
}

bool ClientReadHandler::clientStateServer(const ClientStateServerInfo & /* info */) {
    return false;
}

bool ClientReadHandler::clientStateRoom() {
    return false;
}
