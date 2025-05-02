#pragma once

#include <formats/ClientState.hh>
#include <formats/ServerState.hh>
#include <portable/Allocator.hh>
#include <portable/Array.hh>
#include <portable/network/Address.hh>

class ConnectionState {
public:
    ConnectionState(Allocator &allocator, Array<u8, 32> serverPK);
    virtual ~ConnectionState();
    virtual ConnectionState &reset() = 0;
    virtual ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Address &address, bool &ok) = 0;
    virtual ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Address &address, bool &ok) = 0;

protected:
    Allocator &m_allocator;
    Array<u8, 32> m_serverPK;
};
