#include "DNS.hh"

#include <common/Arena.hh>
#include <common/Bytes.hh>
#include <common/Clock.hh>
#include <portable/Algorithm.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

bool DNS::resolve(const char *name, u32 &address) {
    if (!strcmp(name, "localhost")) {
        address = 127 << 24 | 0 << 16 | 0 << 8 | 1 << 0;
        return true;
    }

    const char *pattern = "%hhu.%hhu.%hhu.%hhu";
    Array<u8, 4> parts;
    if (sscanf(name, pattern, &parts[0], &parts[1], &parts[2], &parts[3]) == 4) {
        address = parts[0] << 24 | parts[1] << 16 | parts[2] << 8 | parts[3] << 0;
        return true;
    }

    if (!m_socket.ok()) {
        m_queries.reset();
        m_socket.open();
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
    s_instance = new (MEM1Arena::Instance(), 0x20) DNS;
    assert(s_instance);
}

DNS *DNS::Instance() {
    return s_instance;
}

DNS::DNS() : m_id(0) {
    for (u32 i = 0; i < m_resolvers.count(); i++) {
        m_resolvers[i].len = sizeof(m_resolvers[i]);
        m_resolvers[i].family = AF_INET;
        m_resolvers[i].port = 53;
    }
    m_resolvers[0].addr = 8 << 24 | 8 << 16 | 8 << 8 | 8 << 0; // Google
    m_resolvers[1].addr = 1 << 24 | 1 << 16 | 1 << 8 | 1 << 0; // Cloudflare
}

bool DNS::readResponse(Response &response) {
    if (m_queries.empty()) {
        return false;
    }

    Array<u8, 512> buffer;
    SOSockAddr resolver;
    resolver.len = sizeof(resolver);
    s32 result = m_socket.recvFrom(buffer.values(), buffer.count(), resolver);
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
    u16 id = Bytes::ReadBE<u16>(buffer.values(), 0x000);
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
    if (Bytes::ReadBE<u16>(buffer.values(), 0x002) != flags) {
        return false;
    }

    u16 qdcount = Bytes::ReadBE<u16>(buffer.values(), 0x004);
    u16 ancount = Bytes::ReadBE<u16>(buffer.values(), 0x006);
    u16 nscount = Bytes::ReadBE<u16>(buffer.values(), 0x008);
    u16 arcount = Bytes::ReadBE<u16>(buffer.values(), 0x00a);
    if (qdcount != 1 || ancount == 0 || nscount != 0 || arcount != 0) {
        return false;
    }
    if (result < 0x00c + qdcount * (0x002 + 0x004) + ancount * (0x002 + 0x00e)) {
        return false;
    }

    u32 ttl = Max<u32>(Bytes::ReadBE<u32>(buffer.values(), result - 0x00a), 5);
    u16 rdlength = Bytes::ReadBE<u16>(buffer.values(), result - 0x006);
    if (rdlength != 4) {
        return false;
    }

    response.expirationTime = Clock::GetMonotonicTicks() + Clock::SecondsToTicks(ttl);
    response.name = m_queries[queryIndex].name;
    response.address = Bytes::ReadBE<u32>(buffer.values(), result - 0x004);

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
    Array<u8, 512> buffer;
    u16 flags = 0;
    flags |= 1 << 8; // RD
    Bytes::WriteBE<u16>(buffer.values(), 0x000, m_id);
    Bytes::WriteBE<u16>(buffer.values(), 0x002, flags);
    Bytes::WriteBE<u16>(buffer.values(), 0x004, 1); // QDCOUNT
    Bytes::WriteBE<u16>(buffer.values(), 0x006, 0); // ANCOUNT
    Bytes::WriteBE<u16>(buffer.values(), 0x008, 0); // NSCOUNT
    Bytes::WriteBE<u16>(buffer.values(), 0x00a, 0); // ARCOUNT

    u32 nameLength, partOffset;
    for (nameLength = 0, partOffset = 0; query.name[nameLength]; nameLength++) {
        if (query.name[nameLength] == '.') {
            if (nameLength == partOffset || nameLength - partOffset >= 64) {
                return false;
            }
            buffer[0x00c + partOffset] = nameLength - partOffset;
            partOffset = nameLength + 1;
        } else {
            buffer[0x00c + 0x001 + nameLength] = query.name[nameLength];
        }
    }
    buffer[0x00c + partOffset] = nameLength - partOffset;
    buffer[0x00c + nameLength + 0x001] = 0;

    Bytes::WriteBE<u16>(buffer.values(), 0x00c + nameLength + 0x002, 1); // QTYPE
    Bytes::WriteBE<u16>(buffer.values(), 0x00c + nameLength + 0x004, 1); // QCLASS

    for (u32 i = 0; i < m_resolvers.count(); i++) {
        m_socket.sendTo(buffer.values(), 0x00c + nameLength + 0x006, m_resolvers[i]);
    }

    if (m_queries.full()) {
        m_queries.popFront();
    }
    m_queries.pushBack(query);

    return true;
}

DNS *DNS::s_instance = nullptr;
