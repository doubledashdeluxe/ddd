#include "ClientReadHandler.hh"

bool ClientReadHandler::clientStateIdle() {
    return false;
}

bool ClientReadHandler::clientStateServer() {
    return false;
}

bool ClientReadHandler::clientStateRoom() {
    return false;
}
