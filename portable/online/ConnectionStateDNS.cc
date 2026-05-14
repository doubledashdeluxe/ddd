#include "ConnectionStateDNS.hh"

#include "portable/Bytes.hh"
#include "portable/online/ConnectionStateKX.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
}

ConnectionStateDNS::ConnectionStateDNS(const ClientPlatform &platform, PublicKey serverPK,
        DNS::Name name, Optional<u16> port)
    : ConnectionState(platform, serverPK)
    , m_name(name)
    , m_port(port) {}

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
    if (!resolve(address)) {
        return *this;
    }

    Key clientEphemeralK;
    m_platform.random.get(clientEphemeralK.values(), clientEphemeralK.count());
    ConnectionState &state = *(new (m_platform.allocator)
                    ConnectionStateKX(m_platform, clientEphemeralK, m_serverPK, address));
    crypto_wipe(clientEphemeralK.values(), clientEphemeralK.count());
    return state;
}

bool ConnectionStateDNS::resolve(Address &address) const {
    address.port = m_port ? *m_port : DefaultPort;

    if (!strcmp(m_name.values(), "localhost")) {
        address.address = 127 << 24 | 0 << 16 | 0 << 8 | 1 << 0;
        return true;
    }

    const char *pattern = "%hhu.%hhu.%hhu.%hhu";
    u8 parts[4];
    if (sscanf(m_name.values(), pattern, &parts[0], &parts[1], &parts[2], &parts[3]) == 4) {
        address.address = Bytes::ReadBE<u32>(parts, 0);
        return true;
    }

    const char *targetName = m_name.values();
    Optional<DNS::Target> target;
    if (!m_port) {
        char name[90];
        snprintf(name, Count(name), "_ddd._udp.%s", m_name.values());
        if (!m_platform.dns.resolveSRV(m_platform.network.resolvers(), name, target)) {
            return false;
        }
    }
    if (target) {
        address.port = target->port;
        targetName = target->name.values();
    }

    Optional<u32> targetAddress;
    if (!m_platform.dns.resolveA(m_platform.network.resolvers(), targetName, targetAddress)) {
        return false;
    }
    if (!targetAddress) {
        return false;
    }

    address.address = *targetAddress;
    return true;
}
