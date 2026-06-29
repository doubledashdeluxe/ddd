#pragma once

#include "portable/crypto/Session.hh"
#include "portable/online/ConnectionState.hh"

class ConnectionStateSession : public ConnectionState {
public:
    ConnectionStateSession(const ClientPlatform &platform, PublicKey serverPK, Address address,
            Session session);
    ~ConnectionStateSession() override;
    Optional<Address> address() const override;
    ConnectionState &reset() override;
    ConnectionState &read(Reader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) override;
    ConnectionState &write(Writer &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) override;

private:
    Address m_address;
    Session m_session;
};
