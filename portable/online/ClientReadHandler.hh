#pragma once

#include "portable/online/ClientStateServerInfo.hh"

class ClientReadHandler {
public:
    virtual bool clientStateIdle();
    virtual bool clientStateServer(const ClientStateServerInfo &info);
    virtual bool clientStateRoom();
    virtual void clientStateError() = 0;
};
