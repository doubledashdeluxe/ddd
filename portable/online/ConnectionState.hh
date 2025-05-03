#pragma once

#include "portable/Array.hh"
#include "portable/network/Address.hh"
#include "portable/online/ClientPlatform.hh"

#include <formats/ClientState.hh>
#include <formats/ServerState.hh>

class ConnectionState {
public:
    ConnectionState(ClientPlatform &platform, Array<u8, 32> serverPK);
    virtual ~ConnectionState();
    virtual ConnectionState &reset() = 0;
    virtual ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Address &address, bool &ok) = 0;
    virtual ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Address &address, bool &ok) = 0;

protected:
    ClientPlatform &m_platform;
    Array<u8, 32> m_serverPK;
};
