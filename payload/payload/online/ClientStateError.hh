#pragma once

#include "payload/online/ClientState.hh"

class ClientStateError : public ClientState {
public:
    ClientStateError(JKRHeap *heap);
    ~ClientStateError() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer() override;
    ClientState &writeStateRoom() override;
    ClientState &writeStateError() override;
};
