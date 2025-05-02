#include "ConnectionStateSession.hh"

#include "payload/crypto/Random.hh"
#include "payload/network/Socket.hh"
#include "payload/online/ConnectionStateKX.hh"

extern "C" {
#include <monocypher/monocypher.h>

#include <assert.h>
}

ConnectionStateSession::ConnectionStateSession(Allocator &allocator, Array<u8, 32> serverPK,
        Address address, Session session)
    : ConnectionState(allocator, serverPK), m_address(address), m_session(session) {}

ConnectionStateSession::~ConnectionStateSession() {}

ConnectionState &ConnectionStateSession::reset() {
    Array<u8, 32> clientEphemeralK;
    Random::Get(clientEphemeralK.values(), clientEphemeralK.count());
    ConnectionState &state = *(new (m_allocator)
                    ConnectionStateKX(m_allocator, clientEphemeralK, m_serverPK, m_address));
    crypto_wipe(clientEphemeralK.values(), clientEphemeralK.count());
    return state;
}

ConnectionState &ConnectionStateSession::read(ServerStateReader &reader, u8 *buffer, u32 size,
        const Address &address, bool &ok) {
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
        Address &address, bool &ok) {
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
