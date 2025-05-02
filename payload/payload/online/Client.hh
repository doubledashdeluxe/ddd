#pragma once

#include "payload/online/ClientState.hh"
#include "payload/online/CubeClientPlatform.hh"

extern "C" {
#include <dolphin/IPSocket.h>
}
#include <jsystem/JKRHeap.hh>
#include <portable/UniquePtr.hh>

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
    CubeClientPlatform m_platform;
    UniquePtr<ClientState> m_state;

    static Client *s_instance;
};
