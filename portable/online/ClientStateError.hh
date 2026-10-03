#pragma once

#include "portable/online/ClientState.hh"

class ClientStateError : public ClientState {
public:
    ClientStateError(const ClientPlatform &platform);
    ~ClientStateError() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &write(const ClientStateIdleWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateUpdateWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &write(const ClientStatePackWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateRoomWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateTeamWriteInfo &writeInfo) override;
    ClientState &write(const ClientStatePollWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateRaceWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateErrorWriteInfo &writeInfo) override;
};
