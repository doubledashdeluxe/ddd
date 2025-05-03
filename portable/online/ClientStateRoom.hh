#pragma once

#include "portable/UniquePtr.hh"
#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

class ClientStateRoom : public ClientState {
public:
    ClientStateRoom(Allocator &allocator);
    ~ClientStateRoom() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;

private:
    UniquePtr<Connection> m_connection;
};
