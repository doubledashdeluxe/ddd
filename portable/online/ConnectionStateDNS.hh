#pragma once

#include "portable/online/ConnectionState.hh"

class ConnectionStateDNS : public ConnectionState {
public:
    ConnectionStateDNS(const ClientPlatform &platform, Array<u8, 32> serverPK, Array<char, 32> name,
            u16 port);
    ~ConnectionStateDNS() override;
    Optional<Address> address() const override;
    ConnectionState &reset() override;
    ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) override;
    ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) override;

private:
    Array<char, 32> m_name;
    u16 m_port;
};
