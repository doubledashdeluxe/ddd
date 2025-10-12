#pragma once

#include "portable/online/ClientStateModeReadInfo.hh"
#include "portable/online/ClientStateServerReadInfo.hh"

class ClientReadHandler {
public:
    virtual bool clientStateIdle();
    virtual bool clientStateServer(const ClientStateServerReadInfo &readInfo);
    virtual bool clientStateMode(const ClientStateModeReadInfo &readInfo);
    virtual void clientStateError() = 0;
};
