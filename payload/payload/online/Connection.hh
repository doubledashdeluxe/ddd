#pragma once

#include "payload/online/ConnectionState.hh"

#include <portable/UniquePtr.hh>
#include <portable/network/Address.hh>
#include <portable/online/ClientPlatform.hh>

class Connection {
public:
    Connection(ClientPlatform &clientPlatform, Array<u8, 32> serverPK, const char *name);
    ~Connection();
    void reset();
    bool read(ServerStateReader &reader, u8 *buffer, u32 size, const Address &address);
    bool write(ClientStateWriter &writer, u8 *buffer, u32 &size, Address &address);

private:
    bool updateState(ConnectionState &nextState);

    UniquePtr<ConnectionState> m_state;
};
