#include "DNS.hh"

#include <common/Algorithm.hh>
#include <common/Arena.hh>
#include <common/Bytes.hh>
#include <common/Clock.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

bool DNS::resolve(const char *name, Array<u8, 4> &address) {
    if (!strcmp(name, "localhost")) {
        address = (u8[4]){127, 0, 0, 1};
        return true;
    }

    const char *pattern = "%hhu.%hhu.%hhu.%hhu";
    if (sscanf(name, pattern, &address[0], &address[1], &address[2], &address[3]) == 4) {
        return true;
    }

    if (!m_socket.ok()) {
        m_queries.reset();
        m_socket.open(Socket::Domain::IPv4);
    }

    Response response;
    if (readResponse(response)) {
        if (!strcmp(response.name.values(), name)) {
            address = response.address;
            return true;
        }
    }

    for (u32 i = 0; i < m_responses.count(); i++) {
        if (strcmp(m_responses[i].name.values(), name)) {
            continue;
        }

        if (m_responses[i].expirationTime < Clock::GetMonotonicTicks()) {
            m_responses.swapRemoveFront(i);
            break;
        } else {
            address = m_responses[i].address;
            return true;
        }
    }

    for (u32 i = 0; i < m_queries.count(); i++) {
        if (strcmp(m_queries[i].name.values(), name)) {
            continue;
        }

        if (m_queries[i].expirationTime < Clock::GetMonotonicTicks()) {
            m_queries.swapRemoveFront(i);
            break;
        } else {
            return false;
        }
    }

    Query query;
    query.id = m_id;
    query.expirationTime = Clock::GetMonotonicTicks() + Clock::SecondsToTicks(2);
    s32 nameLength = snprintf(query.name.values(), query.name.count(), "%s", name);
    if (nameLength < 0 || static_cast<u32>(nameLength) >= query.name.count()) {
        return false;
    }
    if (writeQuery(query)) {
        m_id++;
    }
    return false;
}

void DNS::Init() {
    s_instance = new (MEM2Arena::Instance(), 0x20) DNS;
    assert(s_instance);
}

DNS *DNS::Instance() {
    return s_instance;
}

DNS::DNS() : m_id(0) {
    m_resolvers[0].address = (u8[4]){8, 8, 8, 8}; // Google
    m_resolvers[1].address = (u8[4]){1, 1, 1, 1}; // Cloudflare
    for (u32 i = 0; i < m_resolvers.count(); i++) {
        m_resolvers[i].port = 53;
    }
}

bool DNS::readResponse(Response &response) {
    if (m_queries.empty()) {
        return false;
    }

    Socket::Address resolver;
    s32 result = m_socket.recvFrom(m_buffer.values(), m_buffer.count(), resolver);
    if (result < 0x00c) {
        return false;
    }

    bool hasValidResolver = false;
    for (u32 i = 0; i < m_resolvers.count(); i++) {
        if (m_resolvers[i] == resolver) {
            hasValidResolver = true;
            break;
        }
    }
    if (!hasValidResolver) {
        return false;
    }

    bool hasValidID = false;
    u32 queryIndex;
    u16 id = Bytes::ReadBE<u16>(m_buffer.values(), 0x000);
    for (queryIndex = 0; queryIndex < m_queries.count(); queryIndex++) {
        if (m_queries[queryIndex].id == id) {
            hasValidID = true;
            break;
        }
    }
    if (!hasValidID) {
        return false;
    }

    u16 flags = 0;
    flags |= 1 << 15; // QR
    flags |= 1 << 8;  // RD
    flags |= 1 << 7;  // RA
    if (Bytes::ReadBE<u16>(m_buffer.values(), 0x002) != flags) {
        return false;
    }

    u16 qdcount = Bytes::ReadBE<u16>(m_buffer.values(), 0x004);
    u16 ancount = Bytes::ReadBE<u16>(m_buffer.values(), 0x006);
    u16 nscount = Bytes::ReadBE<u16>(m_buffer.values(), 0x008);
    u16 arcount = Bytes::ReadBE<u16>(m_buffer.values(), 0x00a);
    if (qdcount != 1 || ancount == 0 || nscount != 0 || arcount != 0) {
        return false;
    }
    if (result < 0x00c + qdcount * (0x002 + 0x004) + ancount * (0x002 + 0x00e)) {
        return false;
    }

    u32 ttl = Max<u32>(Bytes::ReadBE<u32>(m_buffer.values(), result - 0x00a), 5);
    u16 rdlength = Bytes::ReadBE<u16>(m_buffer.values(), result - 0x006);
    if (rdlength != 4) {
        return false;
    }

    response.expirationTime = Clock::GetMonotonicTicks() + Clock::SecondsToTicks(ttl);
    response.name = m_queries[queryIndex].name;
    for (u32 i = 0; i < response.address.count(); i++) {
        response.address[i] = m_buffer[result - 0x004 + i];
    }

    m_queries.swapRemoveFront(queryIndex);

    if (ttl != 0) {
        if (m_responses.full()) {
            m_responses.popFront();
        }
        m_responses.pushBack(response);
    }
    return true;
}

bool DNS::writeQuery(const Query &query) {
    u16 flags = 0;
    flags |= 1 << 8; // RD
    Bytes::WriteBE<u16>(m_buffer.values(), 0x000, m_id);
    Bytes::WriteBE<u16>(m_buffer.values(), 0x002, flags);
    Bytes::WriteBE<u16>(m_buffer.values(), 0x004, 1); // QDCOUNT
    Bytes::WriteBE<u16>(m_buffer.values(), 0x006, 0); // ANCOUNT
    Bytes::WriteBE<u16>(m_buffer.values(), 0x008, 0); // NSCOUNT
    Bytes::WriteBE<u16>(m_buffer.values(), 0x00a, 0); // ARCOUNT

    u32 nameLength, partOffset;
    for (nameLength = 0, partOffset = 0; query.name[nameLength]; nameLength++) {
        if (query.name[nameLength] == '.') {
            if (nameLength == partOffset || nameLength - partOffset >= 64) {
                return false;
            }
            m_buffer[0x00c + partOffset] = nameLength - partOffset;
            partOffset = nameLength + 1;
        } else {
            m_buffer[0x00c + 0x001 + nameLength] = query.name[nameLength];
        }
    }
    m_buffer[0x00c + partOffset] = nameLength - partOffset;
    m_buffer[0x00c + nameLength + 0x001] = 0;

    Bytes::WriteBE<u16>(m_buffer.values(), 0x00c + nameLength + 0x002, 1); // QTYPE
    Bytes::WriteBE<u16>(m_buffer.values(), 0x00c + nameLength + 0x004, 1); // QCLASS

    for (u32 i = 0; i < m_resolvers.count(); i++) {
        m_socket.sendTo(m_buffer.values(), 0x00c + nameLength + 0x006, m_resolvers[i]);
    }

    if (m_queries.full()) {
        m_queries.popFront();
    }
    m_queries.pushBack(query);

    return true;
}

DNS *DNS::s_instance = nullptr;
