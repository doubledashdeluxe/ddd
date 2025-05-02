#pragma once

#include "payload/network/UDPSocket.hh"
#include "payload/online/ClientState.hh"
#include "payload/online/Connection.hh"

#include <portable/UniquePtr.hh>

class ClientStateRoom : public ClientState {
public:
    ClientStateRoom(Allocator &allocator);
    ~ClientStateRoom() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;

private:
    UniquePtr<UDPSocket> m_socket;
    UniquePtr<Connection> m_connection;
};
