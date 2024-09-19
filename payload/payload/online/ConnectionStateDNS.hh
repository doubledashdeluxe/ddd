#pragma once

#include "payload/online/ConnectionState.hh"

class ConnectionStateDNS : public ConnectionState {
public:
    ConnectionStateDNS(JKRHeap *heap, Array<u8, 32> serverPK, const char *name);
    ~ConnectionStateDNS() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Socket::Address &address, bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Socket::Address &address, bool &ok) override;

private:
    Array<char, 256> m_name;
    u16 m_port;
};
