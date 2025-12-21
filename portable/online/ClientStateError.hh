#pragma once

#include "portable/online/ClientState.hh"

class ClientStateError : public ClientState {
public:
    ClientStateError(const ClientPlatform &platform);
    ~ClientStateError() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateIdle() override;
    ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;
    ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) override;
    ClientState &writeStateTeam(const ClientStateTeamWriteInfo &writeInfo) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;
    ClientState &writeStateRace(const ClientStateRaceWriteInfo &writeInfo) override;
    ClientState &writeStateError() override;
};
