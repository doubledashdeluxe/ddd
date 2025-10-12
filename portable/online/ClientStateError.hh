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
    ClientState &writeStateError() override;
};
