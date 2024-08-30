#pragma once

#include "payload/online/ClientState.hh"

#include <common/UniquePtr.hh>
#include <jsystem/JKRHeap.hh>

class Client {
public:
    void reset();
    void read(ClientReadHandler &handler);
    void writeStateServer();
    void writeStateRoom();
    void writeStateError();

    static void Init();
    static Client *Instance();

private:
    Client(JKRHeap *heap);

    bool updateState(ClientState &nextState);

    JKRHeap *m_heap;
    UniquePtr<ClientState> m_state;

    static Client *s_instance;
};
