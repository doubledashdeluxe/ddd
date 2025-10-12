#pragma once

#include "payload/HeapAllocator.hh"
#include "payload/network/CubeUDPSocket.hh"

extern "C" {
#include <dolphin/IPSocket.h>
}
#include <jsystem/JKRHeap.hh>
#include <portable/UniquePtr.hh>
#include <portable/online/ClientPlatform.hh>
#include <portable/online/ClientState.hh>

class Client {
public:
    void reset();
    void read(ClientReadHandler &handler);
    void writeStateIdle();
    void writeStateServer(const ClientStateServerWriteInfo &writeInfo);
    void writeStateMode(const ClientStateModeWriteInfo &writeInfo);
    void writeStatePack(const ClientStatePackWriteInfo &writeInfo);
    void writeStateError();

    static void Init(JKRHeap *parentHeap, SOConfig &config);
    static Client *Instance();

private:
    Client(SOConfig &config, JKRHeap *heap);

    bool updateState(ClientState &nextState);

    SOConfig &m_config;
    HeapAllocator m_allocator;
    CubeUDPSocket m_socket;
    ClientPlatform m_platform;
    UniquePtr<ClientState> m_state;

    static Client *s_instance;
};
