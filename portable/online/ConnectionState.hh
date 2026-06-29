#pragma once

#include "portable/Optional.hh"
#include "portable/crypto/Types.hh"
#include "portable/network/Address.hh"
#include "portable/online/ClientPlatform.hh"

#include <formats/Online.hh>

class ConnectionState {
public:
    class Reader {
    public:
        virtual bool isValid(const u8 *buffer, u32 size, u32 &offset) = 0;
        virtual void read(const u8 *buffer, u32 &offset) = 0;
    };

    class Writer {
    public:
        virtual bool write(u8 *buffer, u32 size, u32 &offset) = 0;
    };

    ConnectionState(const ClientPlatform &platform, PublicKey serverPK);
    virtual ~ConnectionState();
    virtual Optional<Address> address() const = 0;
    virtual ConnectionState &reset() = 0;
    virtual ConnectionState &read(Reader &reader, u8 *buffer, u32 size, const Address &address,
            bool &ok) = 0;
    virtual ConnectionState &write(Writer &writer, u8 *buffer, u32 &size, Address &address,
            bool &ok) = 0;

protected:
    const ClientPlatform &m_platform;
    PublicKey m_serverPK;
};
