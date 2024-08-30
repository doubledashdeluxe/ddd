#pragma once

#include "payload/network/Socket.hh"
#include "payload/online/ConnectionState.hh"

#include <common/UniquePtr.hh>
#include <jsystem/JKRHeap.hh>

class Connection {
public:
    Connection(JKRHeap *heap, Array<u8, 32> serverPK, const char *name);
    ~Connection();
    void reset();
    bool read(u8 *buffer, u32 size, const Socket::Address &address);
    bool write(u8 *buffer, u32 &size, Socket::Address &address);

private:
    bool updateState(ConnectionState &nextState);

    JKRHeap *m_heap;
    UniquePtr<ConnectionState> m_state;
};
