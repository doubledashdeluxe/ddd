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

class CubeClient {
public:
    void reset();
    void read(ClientReadHandler &handler);
    void write(const ClientStateIdleWriteInfo &writeInfo);
    void write(const ClientStateServerWriteInfo &writeInfo);
    void write(const ClientStateUpdateWriteInfo &writeInfo);
    void write(const ClientStateModeWriteInfo &writeInfo);
    void write(const ClientStatePackWriteInfo &writeInfo);
    void write(const ClientStateRoomWriteInfo &writeInfo);
    void write(const ClientStateTeamWriteInfo &writeInfo);
    void write(const ClientStatePollWriteInfo &writeInfo);
    void write(const ClientStateRaceWriteInfo &writeInfo);
    void write(const ClientStateErrorWriteInfo &writeInfo);

    static void Init(JKRHeap *heap, SOConfig &config);
    static CubeClient *Instance();

private:
    CubeClient(SOConfig &config, JKRHeap *heap);

    bool updateState(ClientState &nextState);

    SOConfig &m_config;
    HeapAllocator m_allocator;
    CubeUDPSocket m_socket;
    ClientPlatform m_platform;
    UniquePtr<ClientState> m_state;

    static CubeClient *s_instance;
};
