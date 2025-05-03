#include "ConnectionStateDNS.hh"

#include "portable/online/ConnectionStateKX.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
}

ConnectionStateDNS::ConnectionStateDNS(ClientPlatform &platform, Array<u8, 32> serverPK,
        const char *name)
    : ConnectionState(platform, serverPK), m_port(3549) {
    const char *port = strrchr(name, ':');
    if (port && sscanf(port, ":%hu", &m_port) == 1) {
        snprintf(m_name.values(), m_name.count(), "%.*s", static_cast<int>(port - name), name);
    } else {
        snprintf(m_name.values(), m_name.count(), "%s", name);
    }
}

ConnectionStateDNS::~ConnectionStateDNS() {}

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
    if (!m_platform.dns().resolve(m_name.values(), address.address)) {
        return *this;
    }

    Array<u8, 32> clientEphemeralK;
    m_platform.random().get(clientEphemeralK.values(), clientEphemeralK.count());
    address.port = m_port;
    ConnectionState &state = *(new (m_platform.allocator())
                    ConnectionStateKX(m_platform, clientEphemeralK, m_serverPK, address));
    crypto_wipe(clientEphemeralK.values(), clientEphemeralK.count());
    return state;
}
