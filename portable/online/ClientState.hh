#pragma once

#include "portable/online/ClientPlatform.hh"
#include "portable/online/ClientReadHandler.hh"
#include "portable/online/ClientStateModeWriteInfo.hh"
#include "portable/online/ClientStatePackWriteInfo.hh"
#include "portable/online/ClientStateRoomWriteInfo.hh"
#include "portable/online/ClientStateServerWriteInfo.hh"
#include "portable/online/ClientStateTeamWriteInfo.hh"

class ClientState {
public:
    ClientState(const ClientPlatform &platform);
    virtual ~ClientState();
    virtual bool needsSockets() = 0;
    virtual ClientState &read(ClientReadHandler &handler) = 0;
    virtual ClientState &writeStateIdle();
    virtual ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo);
    virtual ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo);
    virtual ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo);
    virtual ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo);
    virtual ClientState &writeStateTeam(const ClientStateTeamWriteInfo &writeInfo);
    virtual ClientState &writeStateError();

protected:
    const ClientPlatform &m_platform;
};
