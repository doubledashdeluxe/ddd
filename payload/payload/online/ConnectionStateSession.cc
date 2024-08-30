#include "ConnectionStateSession.hh"

#include "payload/online/ConnectionStateKX.hh"

ConnectionStateSession::ConnectionStateSession(JKRHeap *heap, Array<u8, 32> serverPK,
        Socket::Address address, Session session)
    : ConnectionState(heap, serverPK), m_address(address), m_session(session) {}

ConnectionStateSession::~ConnectionStateSession() {}

ConnectionState &ConnectionStateSession::reset() {
    return *(new (m_heap, 0x4) ConnectionStateKX(m_heap, m_serverPK, m_address));
}

ConnectionState &ConnectionStateSession::read(u8 *buffer, u32 size, const Socket::Address &address,
        bool &ok) {
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

    // TODO read the decrypted message

    return *this;
}

ConnectionState &ConnectionStateSession::write(u8 *buffer, u32 &size, Socket::Address &address,
        bool &ok) {
    ok = size >= Session::MACSize + Session::NonceSize;

    if (!ok) {
        return *this;
    }

    size -= Session::MACSize + Session::NonceSize;
    size = 0;
    // TODO write the message to encrypt

    u8 *mac = buffer + size;
    u8 *nonce = mac + Session::MACSize;
    m_session.write(buffer, size, mac, nonce);
    address = m_address;
    return *this;
}
