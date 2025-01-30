#pragma once

#include "payload/online/ClientState.hh"

#include <common/UniquePtr.hh>
extern "C" {
#include <dolphin/IPSocket.h>
}
#include <jsystem/JKRHeap.hh>

class Client {
public:
    void reset();
    void read(ClientReadHandler &handler);
    void writeStateIdle();
    void writeStateServer();
    void writeStateRoom();
    void writeStateError();

    static void Init(JKRHeap *parentHeap, SOConfig &config);
    static Client *Instance();

private:
    Client(SOConfig &config, JKRHeap *heap);

    bool updateState(ClientState &nextState);

    SOConfig &m_config;
    JKRHeap *m_heap;
    UniquePtr<ClientState> m_state;

    static Client *s_instance;
};
