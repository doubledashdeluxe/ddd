#pragma once

#include "payload/crypto/Session.hh"
#include "payload/online/ConnectionState.hh"

class ConnectionStateSession : public ConnectionState {
public:
    ConnectionStateSession(JKRHeap *heap, Array<u8, 32> serverPK, Socket::Address address,
            Session session);
    ~ConnectionStateSession() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Socket::Address &address, bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Socket::Address &address, bool &ok) override;

private:
    Socket::Address m_address;
    Session m_session;
};
