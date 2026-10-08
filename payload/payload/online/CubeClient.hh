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
#include <portable/online/ReplayManager.hh>

class CubeClient {
public:
    void reset();
    void setReplay(const ReplayManager::Replay &replay, u32 clientIndex);
    void read(ClientReadHandler &handler);

    template <typename W>
    void write(const W &writeInfo) {
        while (updateState(m_state->write(writeInfo))) {}
    }

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
