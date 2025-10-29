#pragma once

#include "portable/Optional.hh"
#include "portable/crypto/Types.hh"
#include "portable/network/Address.hh"
#include "portable/online/ClientPlatform.hh"

#include <formats/Online.hh>

class ConnectionState {
public:
    ConnectionState(const ClientPlatform &platform, PublicKey serverPK);
    virtual ~ConnectionState();
    virtual Optional<Address> address() const = 0;
    virtual ConnectionState &reset() = 0;
    virtual ConnectionState &read(ServerStateReader &reader, u8 *buffer, u32 size,
            const Address &address, bool &ok) = 0;
    virtual ConnectionState &write(ClientStateWriter &writer, u8 *buffer, u32 &size,
            Address &address, bool &ok) = 0;

protected:
    const ClientPlatform &m_platform;
    PublicKey m_serverPK;
};
