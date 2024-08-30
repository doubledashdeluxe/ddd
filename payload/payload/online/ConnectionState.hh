#pragma once

#include "payload/network/Socket.hh"

#include <common/Array.hh>
#include <jsystem/JKRHeap.hh>

class ConnectionState {
public:
    ConnectionState(JKRHeap *heap, Array<u8, 32> serverPK);
    virtual ~ConnectionState();
    virtual ConnectionState &reset() = 0;
    virtual ConnectionState &read(u8 *buffer, u32 size, const Socket::Address &address,
            bool &ok) = 0;
    virtual ConnectionState &write(u8 *buffer, u32 &size, Socket::Address &address, bool &ok) = 0;

protected:
    JKRHeap *m_heap;
    Array<u8, 32> m_serverPK;
};
