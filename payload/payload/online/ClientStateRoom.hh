#pragma once

#include "payload/network/UDPSocket.hh"
#include "payload/online/ClientState.hh"

#include <portable/UniquePtr.hh>
#include <portable/online/Connection.hh>

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
