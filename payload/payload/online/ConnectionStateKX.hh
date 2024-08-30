#pragma once

#include "payload/crypto/KX.hh"
#include "payload/online/ConnectionState.hh"

class ConnectionStateKX : public ConnectionState {
public:
    ConnectionStateKX(JKRHeap *heap, Array<u8, 32> serverPK, Socket::Address address);
    ~ConnectionStateKX() override;
    ConnectionState &reset() override;
    ConnectionState &read(u8 *buffer, u32 size, const Socket::Address &address, bool &ok) override;
    ConnectionState &write(u8 *buffer, u32 &size, Socket::Address &address, bool &ok) override;

private:
    Socket::Address m_address;
    Array<u8, KX::M1Size> m_m1;
    KX::ClientState m_clientState;
};
