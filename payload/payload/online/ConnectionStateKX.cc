#include "ConnectionStateKX.hh"

#include "payload/network/Socket.hh"
#include "payload/online/ClientK.hh"
#include "payload/online/ConnectionStateSession.hh"

extern "C" {
#include <assert.h>
}

ConnectionStateKX::ConnectionStateKX(JKRHeap *heap, Array<u8, 32> serverPK, SOSockAddr address)
    : ConnectionState(heap, serverPK), m_address(address), m_clientState(ClientK::Get(), serverPK) {
}

ConnectionStateKX::~ConnectionStateKX() {}

ConnectionState &ConnectionStateKX::reset() {
    return *this;
}

ConnectionState &ConnectionStateKX::read(ServerStateReader & /* reader */, u8 *buffer, u32 size,
        const SOSockAddr &address, bool &ok) {
    ok = address == m_address;

    if (!ok) {
        return *this;
    }

    if (size != KX::M2Size) {
        return *this;
    }

    m_clientState.setM2(buffer);
    return *this;
}

ConnectionState &ConnectionStateKX::write(ClientStateWriter & /* writer */, u8 *buffer, u32 &size,
        SOSockAddr &address, bool &ok) {
    assert(size >= KX::M1Size);

    ok = false;

    const Session *session = nullptr;
    if (!m_clientState.update()) {
        session = m_clientState.clientSession();
        if (!session) {
            m_clientState = KX::ClientState(ClientK::Get(), m_serverPK);
        }
    }

    if (session) {
        return *(new (m_heap, 0x4) ConnectionStateSession(m_heap, m_serverPK, m_address, *session));
    }

    ok = m_clientState.getM1(buffer);
    if (!ok) {
        return *this;
    }

    size = KX::M1Size;
    address = m_address;
    return *this;
}
