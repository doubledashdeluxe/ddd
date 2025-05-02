#pragma once

#include "payload/online/ClientState.hh"

class ClientStateIdle : public ClientState {
public:
    ClientStateIdle(ClientPlatform &platform);
    ~ClientStateIdle() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateIdle() override;
    ClientState &writeStateServer() override;
};
