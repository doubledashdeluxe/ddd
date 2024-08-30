#pragma once

#include "payload/network/UDPSocket.hh"
#include "payload/online/ClientState.hh"
#include "payload/online/Connection.hh"

#include <common/UniquePtr.hh>

class ClientStateRoom : public ClientState {
public:
    ClientStateRoom(JKRHeap *heap);
    ~ClientStateRoom() override;
    ClientState &read(ClientReadHandler &handler) override;

private:
    UniquePtr<UDPSocket> m_socket;
    UniquePtr<Connection> m_connection;
};
