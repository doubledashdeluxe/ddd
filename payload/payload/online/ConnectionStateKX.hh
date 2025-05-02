#pragma once

#include "payload/online/ConnectionState.hh"

#include <portable/crypto/KX.hh>

class ConnectionStateKX : public ConnectionState {
public:
    ConnectionStateKX(ClientPlatform &platform, const Array<u8, 32> &clientEphemeralK,
            Array<u8, 32> serverPK, Address address);
    ~ConnectionStateKX() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) override;

private:
    Address m_address;
    KX::ClientState m_clientState;
};
