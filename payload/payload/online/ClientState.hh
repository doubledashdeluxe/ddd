#pragma once

#include "payload/online/ClientReadHandler.hh"

#include <portable/Allocator.hh>

class ClientState {
public:
    ClientState(Allocator &allocator);
    virtual ~ClientState();
    virtual bool needsSockets() = 0;
    virtual ClientState &read(ClientReadHandler &handler) = 0;
    virtual ClientState &writeStateIdle();
    virtual ClientState &writeStateServer();
    virtual ClientState &writeStateRoom();
    virtual ClientState &writeStateError();

protected:
    Allocator &m_allocator;
};
