#pragma once

#include "portable/online/ClientState.hh"

class ClientStateIdle : public ClientState {
public:
    ClientStateIdle(const ClientPlatform &platform);
    ~ClientStateIdle() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateIdle() override;
    ClientState &writeStateServer() override;
};
