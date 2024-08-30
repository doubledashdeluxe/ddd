#include "ConnectionStateDNS.hh"

#include "payload/network/DNS.hh"
#include "payload/online/ConnectionStateKX.hh"

extern "C" {
#include <stdio.h>
#include <string.h>
}

ConnectionStateDNS::ConnectionStateDNS(JKRHeap *heap, Array<u8, 32> serverPK, const char *name)
    : ConnectionState(heap, serverPK), m_port(3549) {
    const char *port = strrchr(name, ':');
    if (port && sscanf(port, ":%hu", &m_port) == 1) {
        snprintf(m_name.values(), m_name.count(), "%.*s", static_cast<s32>(port - name), name);
    } else {
        snprintf(m_name.values(), m_name.count(), "%s", name);
    }
}

ConnectionStateDNS::~ConnectionStateDNS() {}

ConnectionState &ConnectionStateDNS::reset() {
    return *this;
}

ConnectionState &ConnectionStateDNS::read(u8 * /* buffer */, u32 /* size */,
        const Socket::Address & /* address */, bool &ok) {
    ok = false;

    return *this;
}

ConnectionState &ConnectionStateDNS::write(u8 * /* buffer */, u32 & /* size */,
        Socket::Address & /* address */, bool &ok) {
    ok = false;

    Socket::Address address;
    if (!DNS::Instance()->resolve(m_name.values(), address.address)) {
        return *this;
    }

    address.port = m_port;
    return *(new (m_heap, 0x4) ConnectionStateKX(m_heap, m_serverPK, address));
}
