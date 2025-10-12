#pragma once

#include "native/NativeAllocator.hh"
#include "native/crypto/NativeRandom.hh"
#include "native/network/DNSFakeUDPSocket.hh"
#include "native/network/FakeDNS.hh"
#include "native/network/FakeNetwork.hh"
#include "native/network/OnlineFakeUDPSocket.hh"
#include "native/online/FakeServerManager.hh"
#include "native/online/ServerPlatform.hh"

#include <portable/online/ClientPlatform.hh>
#include <portable/online/ClientState.hh>

#include <memory>
#include <vector>

class FakeClient : private ClientReadHandler {
public:
    FakeClient(const std::map<std::vector<std::string>, u32> &dnsServers,
            const Array<u8, 32> &serverK, const std::vector<u8> &data,
            const Ring<ServerManager::Server, ServerManager::MaxServerCount> &servers,
            const Array<u8, 32> &clientK);
    ~FakeClient();

    void read();
    bool write();

private:
    typedef ClientState &(FakeClient::*Writer)();

    bool clientStateIdle() override;
    bool clientStateServer(const ClientStateServerReadInfo &readInfo) override;
    bool clientStateMode(const ClientStateModeReadInfo &readInfo) override;
    void clientStateError() override;

    ClientState &writeStateServer();
    ClientState &writeStateMode();

    bool updateState(ClientState &nextState);

    NativeAllocator m_allocator;
    NativeRandom m_random;
    FakeNetwork m_network;
    DNSFakeUDPSocket m_dnsSocket;
    FakeDNS m_dns;
    ServerPlatform m_serverPlatform;
    OnlineFakeUDPSocket m_socket;
    FakeServerManager m_serverManager;
    ClientPlatform m_platform;
    std::unique_ptr<ClientState> m_state;
    Writer m_writer = &FakeClient::writeStateServer;
};
