#pragma once

#include "payload/network/UDPSocket.hh"
#include "payload/online/ClientState.hh"
#include "payload/online/Connection.hh"
#include "payload/online/ServerManager.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>

class ClientStateServer : public ClientState {
public:
    ClientStateServer(JKRHeap *heap, UDPSocket *socket, Array<u8, 512> *buffer);
    ~ClientStateServer() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer() override;

private:
    void checkConnections();
    void checkSocket();

    UniquePtr<UDPSocket> m_socket;
    UniquePtr<Array<u8, 512>> m_buffer;
    Ring<UniquePtr<Connection>, ServerManager::MaxServerCount> m_connections;
    u32 m_index;
};
