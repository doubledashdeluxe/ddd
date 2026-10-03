#pragma once

#include "portable/online/ClientState.hh"

class ClientStateIdle : public ClientState {
public:
    ClientStateIdle(const ClientPlatform &platform);
    ~ClientStateIdle() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &write(const ClientStateIdleWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateServerWriteInfo &writeInfo) override;
};
