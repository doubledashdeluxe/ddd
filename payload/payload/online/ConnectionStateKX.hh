#pragma once

#include "payload/online/ConnectionState.hh"

#include <portable/crypto/KX.hh>

class ConnectionStateKX : public ConnectionState {
public:
    ConnectionStateKX(Allocator &allocator, const Array<u8, 32> &clientEphemeralK,
            Array<u8, 32> serverPK, SOSockAddr address);
    ~ConnectionStateKX() override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const SOSockAddr &address, bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size, SOSockAddr &address,
            bool &ok) override;

private:
    SOSockAddr m_address;
    KX::ClientState m_clientState;
};
