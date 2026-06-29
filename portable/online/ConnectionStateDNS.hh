#pragma once

#include "portable/crypto/Types.hh"
#include "portable/online/ConnectionState.hh"

class ConnectionStateDNS : public ConnectionState {
public:
    ConnectionStateDNS(const ClientPlatform &platform, PublicKey serverPK, DNS::Name name,
            Optional<u16> port);
    ~ConnectionStateDNS() override;
    Optional<Address> address() const override;
    ConnectionState &reset() override;
    ConnectionState &read(Reader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) override;
    ConnectionState &write(Writer &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) override;

private:
    bool resolve(Address &address) const;

    DNS::Name m_name;
    Optional<u16> m_port;
};
