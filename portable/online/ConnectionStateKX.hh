#pragma once

#include "portable/crypto/KX.hh"
#include "portable/online/ConnectionState.hh"
#include "portable/online/CookieSize.hh"

class ConnectionStateKX : public ConnectionState {
public:
    ConnectionStateKX(const ClientPlatform &platform, const Array<u8, 32> &clientEphemeralK,
            Array<u8, 32> serverPK, Address address);
    ~ConnectionStateKX() override;
    Optional<Address> address() const override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) override;

private:
    Address m_address;
    Array<u8, CookieSize> m_cookie;
    KX::ClientState m_clientState;
};
