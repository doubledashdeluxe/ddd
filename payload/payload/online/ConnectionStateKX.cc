#include "ConnectionStateKX.hh"

#include "payload/crypto/Random.hh"
#include "payload/online/ConnectionStateSession.hh"

#include <common/Log.hh>
extern "C" {
#include <monocypher/monocypher.h>

#include <stdio.h>
#include <string.h>
}

ConnectionStateKX::ConnectionStateKX(JKRHeap *heap, Array<u8, 32> serverPK, Socket::Address address)
    : ConnectionState(heap, serverPK), m_address(address) {
    Array<u8, 32> clientK;
    Random::Get(clientK.values(), clientK.count());
    KX::IK1(clientK, m_serverPK, m_m1.values(), m_clientState);
    crypto_wipe(clientK.values(), clientK.count());
}

ConnectionStateKX::~ConnectionStateKX() {}

ConnectionState &ConnectionStateKX::reset() {
    return *this;
}

ConnectionState &ConnectionStateKX::read(u8 *buffer, u32 size, const Socket::Address &address,
        bool &ok) {
    ok = address == m_address;

    if (!ok) {
        return *this;
    }

    if (size != KX::M2Size) {
        return *this;
    }

    Session session;
    if (!KX::IK3(m_clientState, buffer, session)) {
        return *this;
    }
    // TODO don't print this (duh)
    Array<char, 128> k;
    for (u32 i = 0; i < 32; i++) {
        snprintf(k.values() + 2 * i, 3, "%02x", session.m_readK.values()[i]);
    }
    DEBUG("Read key: %s", k.values());
    for (u32 i = 0; i < 32; i++) {
        snprintf(k.values() + 2 * i, 3, "%02x", session.m_writeK.values()[i]);
    }
    DEBUG("Write key: %s", k.values());

    return *(new (m_heap, 0x4) ConnectionStateSession(m_heap, m_serverPK, m_address, session));
}

ConnectionState &ConnectionStateKX::write(u8 *buffer, u32 &size, Socket::Address &address,
        bool &ok) {
    ok = size >= m_m1.count();

    if (!ok) {
        return *this;
    }

    memcpy(buffer, m_m1.values(), m_m1.count());
    size = m_m1.count();
    address = m_address;
    return *this;
}
