#pragma once

#include "payload/online/ClientReadHandler.hh"

#include <jsystem/JKRHeap.hh>

class ClientState {
public:
    ClientState(JKRHeap *heap);
    virtual ~ClientState();
    virtual bool needsSockets() = 0;
    virtual ClientState &read(ClientReadHandler &handler) = 0;
    virtual ClientState &writeStateIdle();
    virtual ClientState &writeStateServer();
    virtual ClientState &writeStateRoom();
    virtual ClientState &writeStateError();

protected:
    JKRHeap *m_heap;
};
