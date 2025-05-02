#pragma once

#include "payload/online/ClientReadHandler.hh"

#include <portable/online/ClientPlatform.hh>

class ClientState {
public:
    ClientState(ClientPlatform &platform);
    virtual ~ClientState();
    virtual bool needsSockets() = 0;
    virtual ClientState &read(ClientReadHandler &handler) = 0;
    virtual ClientState &writeStateIdle();
    virtual ClientState &writeStateServer();
    virtual ClientState &writeStateRoom();
    virtual ClientState &writeStateError();

protected:
    ClientPlatform &m_platform;
};
