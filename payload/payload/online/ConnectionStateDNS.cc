#include "ConnectionStateDNS.hh"

#include "payload/crypto/Random.hh"
#include "payload/network/CubeDNS.hh"
#include "payload/online/ConnectionStateKX.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
}

ConnectionStateDNS::ConnectionStateDNS(Allocator &allocator, Array<u8, 32> serverPK,
        const char *name)
    : ConnectionState(allocator, serverPK), m_port(3549) {
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
        u32 /* size */, const SOSockAddr & /* address */, bool &ok) {
    ok = false;

    return *this;
}

ConnectionState &ConnectionStateDNS::write(ClientStateWriter & /* writer */, u8 * /* buffer */,
        u32 & /* size */, SOSockAddr & /* address */, bool &ok) {
    ok = false;

    SOSockAddr address;
    address.len = sizeof(address);
    address.family = AF_INET;
    if (!CubeDNS::Instance()->resolve(m_name.values(), address.addr)) {
        return *this;
    }

    Array<u8, 32> clientEphemeralK;
    Random::Get(clientEphemeralK.values(), clientEphemeralK.count());
    address.port = m_port;
    ConnectionState &state = *(new (m_allocator)
                    ConnectionStateKX(m_allocator, clientEphemeralK, m_serverPK, address));
    crypto_wipe(clientEphemeralK.values(), clientEphemeralK.count());
    return state;
}
