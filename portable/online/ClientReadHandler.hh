#pragma once

#include "portable/online/ClientStateModeReadInfo.hh"
#include "portable/online/ClientStatePackReadInfo.hh"
#include "portable/online/ClientStatePollReadInfo.hh"
#include "portable/online/ClientStateRaceReadInfo.hh"
#include "portable/online/ClientStateRoomReadInfo.hh"
#include "portable/online/ClientStateServerReadInfo.hh"
#include "portable/online/ClientStateTeamReadInfo.hh"
#include "portable/online/ClientStateUpdateReadInfo.hh"

class ClientReadHandler {
public:
    virtual bool clientStateIdle();
    virtual bool clientStateServer(const ClientStateServerReadInfo &readInfo);
    virtual bool clientStateUpdate(const ClientStateUpdateReadInfo &readInfo);
    virtual bool clientStateMode(const ClientStateModeReadInfo &readInfo);
    virtual bool clientStatePack(const ClientStatePackReadInfo &readInfo);
    virtual bool clientStateRoom(const ClientStateRoomReadInfo &readInfo);
    virtual bool clientStateTeam(const ClientStateTeamReadInfo &readInfo);
    virtual bool clientStatePoll(const ClientStatePollReadInfo &readInfo);
    virtual bool clientStateRace(const ClientStateRaceReadInfo &readInfo);
    virtual void clientStateError() = 0;
};
