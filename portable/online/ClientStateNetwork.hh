#pragma once

#include "portable/online/ClientState.hh"

class ClientStateNetwork : public ClientState {
public:
    ClientStateNetwork(const ClientPlatform &platform);
    ~ClientStateNetwork() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &write(const ClientStateNetworkWriteInfo &writeInfo) override;
    ClientState &write(const ClientStateServerWriteInfo &writeInfo) override;
};
