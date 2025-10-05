#include "ConnectionStateKX.hh"

#include "portable/online/ConnectionStateSession.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <assert.h>
#include <string.h>
}

ConnectionStateKX::ConnectionStateKX(const ClientPlatform &platform,
        const Array<u8, 32> &clientEphemeralK, Array<u8, 32> serverPK, Address address)
    : ConnectionState(platform, serverPK), m_address(address), m_cookie(0),
      m_clientState(platform.clientK, clientEphemeralK, serverPK) {}

ConnectionStateKX::~ConnectionStateKX() {}

Optional<Address> ConnectionStateKX::address() const {
    return m_address;
}

ConnectionState &ConnectionStateKX::reset() {
    return *this;
}

ConnectionState &ConnectionStateKX::read(ServerStateReader & /* reader */, u8 *buffer, u32 size,
        const Address &address, bool &ok) {
    ok = address == m_address;

    if (!ok) {
        return *this;
    }

    if (size == m_cookie.count()) {
        memcpy(m_cookie.values(), buffer, m_cookie.count());
        return *this;
    }

    if (size != KX::M2Size) {
        return *this;
    }

    m_clientState.setM2(buffer);
    return *this;
}

ConnectionState &ConnectionStateKX::write(ClientStateWriter & /* writer */, u8 *buffer, u32 &size,
        Address &address, bool &ok) {
    assert(size >= m_cookie.count() + KX::M1Size);

    ok = false;

    const Session *session = nullptr;
    if (!m_clientState.update()) {
        session = m_clientState.clientSession();
        if (!session) {
            Array<u8, 32> clientEphemeralK;
            m_platform.random.get(clientEphemeralK.values(), clientEphemeralK.count());
            m_clientState = KX::ClientState(m_platform.clientK, clientEphemeralK, m_serverPK);
            crypto_wipe(clientEphemeralK.values(), clientEphemeralK.count());
        }
    }

    if (session) {
        return *(new (m_platform.allocator)
                        ConnectionStateSession(m_platform, m_serverPK, m_address, *session));
    }

    ok = m_clientState.getM1(buffer + m_cookie.count());
    if (!ok) {
        return *this;
    }

    memcpy(buffer, m_cookie.values(), m_cookie.count());
    size = m_cookie.count() + KX::M1Size;
    address = m_address;
    return *this;
}
