#include "ConnectionStateSession.hh"

#include "payload/online/ConnectionStateKX.hh"

extern "C" {
#include <assert.h>
}

ConnectionStateSession::ConnectionStateSession(JKRHeap *heap, Array<u8, 32> serverPK,
        Socket::Address address, Session session)
    : ConnectionState(heap, serverPK), m_address(address), m_session(session) {}

ConnectionStateSession::~ConnectionStateSession() {}

ConnectionState &ConnectionStateSession::reset() {
    return *(new (m_heap, 0x4) ConnectionStateKX(m_heap, m_serverPK, m_address));
}

ConnectionState &ConnectionStateSession::read(ServerStateReader &reader, u8 *buffer, u32 size,
        const Socket::Address &address, bool &ok) {
    ok = address == m_address;

    if (!ok) {
        return *this;
    }

    if (size < Session::MACSize + Session::NonceSize) {
        return *this;
    }

    size -= Session::MACSize + Session::NonceSize;
    const u8 *mac = buffer + size;
    const u8 *nonce = mac + Session::MACSize;
    if (!m_session.read(buffer, size, mac, nonce)) {
        return *this;
    }

    u32 offset = 0;
    if (!reader.isValid(buffer, size, offset)) {
        return *this;
    }
    offset = 0;
    reader.read(buffer, offset);

    return *this;
}

ConnectionState &ConnectionStateSession::write(ClientStateWriter &writer, u8 *buffer, u32 &size,
        Socket::Address &address, bool &ok) {
    assert(size >= Session::MACSize + Session::NonceSize);

    ok = true;

    size -= Session::MACSize + Session::NonceSize;
    u32 offset = 0;
    assert(writer.write(buffer, size, offset));
    size = offset;

    u8 *mac = buffer + size;
    u8 *nonce = mac + Session::MACSize;
    m_session.write(buffer, size, mac, nonce);
    size += Session::MACSize + Session::NonceSize;
    address = m_address;
    return *this;
}
