#pragma once

#include "payload/online/ConnectionState.hh"

extern "C" {
#include <dolphin/IPSocket.h>
}
#include <portable/Allocator.hh>
#include <portable/UniquePtr.hh>

class Connection {
public:
    Connection(Allocator &allocator, Array<u8, 32> serverPK, const char *name);
    ~Connection();
    void reset();
    bool read(ServerStateReader &reader, u8 *buffer, u32 size, const SOSockAddr &address);
    bool write(ClientStateWriter &writer, u8 *buffer, u32 &size, SOSockAddr &address);

private:
    bool updateState(ConnectionState &nextState);

    UniquePtr<ConnectionState> m_state;
};
