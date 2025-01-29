#pragma once

#include "payload/crypto/Session.hh"
#include "payload/online/ConnectionState.hh"

class ConnectionStateSession : public ConnectionState {
public:
    ConnectionStateSession(JKRHeap *heap, Array<u8, 32> serverPK, SOSockAddr address,
            Session session);
    ~ConnectionStateSession() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const SOSockAddr &address, bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size, SOSockAddr &address,
            bool &ok) override;

private:
    SOSockAddr m_address;
    Session m_session;
};
