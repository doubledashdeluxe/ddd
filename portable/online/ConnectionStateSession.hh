#pragma once

#include "portable/crypto/Session.hh"
#include "portable/online/ConnectionState.hh"

class ConnectionStateSession : public ConnectionState {
public:
    ConnectionStateSession(const ClientPlatform &platform, Array<u8, 32> serverPK, Address address,
            Session session);
    ~ConnectionStateSession() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) override;

private:
    Address m_address;
    Session m_session;
};
