#include "ConnectionStateDNS.hh"

#include "portable/online/ConnectionStateKX.hh"

extern "C" {
#include <monocypher/monocypher.h>
}

ConnectionStateDNS::ConnectionStateDNS(const ClientPlatform &platform, Array<u8, 32> serverPK,
        Array<char, 32> name, u16 port)
    : ConnectionState(platform, serverPK), m_name(name), m_port(port) {}

ConnectionStateDNS::~ConnectionStateDNS() {}

Optional<Address> ConnectionStateDNS::address() const {
    return Optional<Address>();
}

ConnectionState &ConnectionStateDNS::reset() {
    return *this;
}

ConnectionState &ConnectionStateDNS::read(ServerStateReader & /* reader */, u8 * /* buffer */,
        u32 /* size */, const Address & /* address */, bool &ok) {
    ok = false;

    return *this;
}

ConnectionState &ConnectionStateDNS::write(ClientStateWriter & /* writer */, u8 * /* buffer */,
        u32 & /* size */, Address & /* address */, bool &ok) {
    ok = false;

    Address address;
    if (!m_platform.dns.resolve(m_name.values(), address.address)) {
        return *this;
    }

    Array<u8, 32> clientEphemeralK;
    m_platform.random.get(clientEphemeralK.values(), clientEphemeralK.count());
    address.port = m_port;
    ConnectionState &state = *(new (m_platform.allocator)
                    ConnectionStateKX(m_platform, clientEphemeralK, m_serverPK, address));
    crypto_wipe(clientEphemeralK.values(), clientEphemeralK.count());
    return state;
}
