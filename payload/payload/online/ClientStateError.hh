#pragma once

#include "payload/online/ClientState.hh"

class ClientStateError : public ClientState {
public:
    ClientStateError(JKRHeap *heap);
    ~ClientStateError() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateIdle() override;
    ClientState &writeStateServer() override;
    ClientState &writeStateRoom() override;
    ClientState &writeStateError() override;
};
