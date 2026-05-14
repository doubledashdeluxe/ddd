// clang-format off
//
// Resources:
// - https://datatracker.ietf.org/doc/html/rfc1035
//
// clang-format on

#include "DNS.hh"

#include "portable/Algorithm.hh"

bool DNS::resolveA(const Resolvers &resolvers, const char *name, Optional<u32> &address) {
    return resolve(resolvers, name, address, m_aEntries, A);
}

bool DNS::resolveSRV(const Resolvers &resolvers, const char *name, Optional<Target> &target) {
    return resolve(resolvers, name, target, m_srvEntries, SRV);
}

bool DNS::ReadName(const u8 *buffer, u32 size, u32 &offset, Name &name, u32 &nameLength) {
    nameLength = 0;
    for (u32 partOffset = offset;;) {
        if (partOffset >= size) {
            return false;
        }

        u8 partLength = buffer[partOffset++];
        offset = Max(offset, partOffset);
        if ((partLength & 0xc0) == 0xc0) {
            if (partOffset >= size) {
                return false;
            }

            size = partOffset - 1;
            offset = Max(offset, partOffset + 1);
            partOffset = (partLength & ~0xc0) << 8 | buffer[partOffset++] << 0;
            continue;
        }

        if (partLength >= 64) {
            return false;
        }

        if (partLength == 0) {
            if (nameLength == 0) {
                return false;
            }

            name[--nameLength] = '\0';
            return true;
        }

        if (nameLength + partLength >= name.count()) {
            return false;
        }

        if (partOffset + partLength >= size) {
            return false;
        }

        for (u32 i = 0; i < partLength; i++) {
            char c = buffer[partOffset + i];
            if (c == '\0' || c == '.') {
                return false;
            }

            name[nameLength + i] = c;
        }
        nameLength += partLength;
        name[nameLength++] = '.';
        partOffset += partLength;
    }
}

bool DNS::WriteName(u8 *buffer, u32 size, u32 &offset, const Name &name, u32 &nameLength) {
    u32 partOffset;
    for (nameLength = 0, partOffset = 0; name[nameLength]; nameLength++) {
        if (name[nameLength] == '.') {
            if (nameLength == partOffset || nameLength - partOffset >= 64) {
                return false;
            }

            buffer[offset + partOffset] = nameLength - partOffset;
            partOffset = nameLength + 1;
        } else {
            if (offset + 0x001 + nameLength >= size) {
                return false;
            }

            buffer[offset + 0x001 + nameLength] = name[nameLength];
        }
    }
    buffer[offset + partOffset] = nameLength - partOffset;
    buffer[offset + nameLength + 0x001] = 0;
    offset += nameLength + 0x002;
    return true;
}

DNS::DNS(UDPSocket &socket) : m_socket(socket), m_id(0) {
    for (u32 i = 0; i < m_resolvers.count(); i++) {
        m_resolvers[i] = 127 << 24 | 0 << 16 | 0 << 8 | 1 << 0;
    }
}

DNS::~DNS() {}

void DNS::checkSocket() {
    if (!m_socket.ok()) {
        resetEntries();
        m_socket.open();
    }
}

void DNS::checkResolvers(const Resolvers &resolvers) {
    if (resolvers != m_resolvers) {
        resetEntries();
        m_resolvers = resolvers;
    }
}

void DNS::resetEntries() {
    ResetQueries(m_aEntries);
    ResetQueries(m_srvEntries);
}

bool DNS::readResponse(s64 now) {
    u8 buffer[512];
    Address resolver;
    s32 result = m_socket.recvFrom(buffer, Count(buffer), resolver);
    if (result < 0x00c) {
        return false;
    }

    if (!hasResolver(resolver)) {
        return false;
    }

    u16 id = Bytes::ReadBE<u16>(buffer, 0x000);

    u16 qdcount = Bytes::ReadBE<u16>(buffer, 0x004);
    u16 ancount = Bytes::ReadBE<u16>(buffer, 0x006);
    u16 nscount = Bytes::ReadBE<u16>(buffer, 0x008);
    if (qdcount != 1 || nscount != 0) {
        return false;
    }

    u16 flags = 0;
    flags |= 1 << 15;                // QR
    flags |= 1 << 8;                 // RD
    flags |= 1 << 7;                 // RA
    flags |= (ancount ? 0 : 3) << 0; // RCODE
    if (Bytes::ReadBE<u16>(buffer, 0x002) != flags) {
        return false;
    }

    u32 size = result, offset = 0x00c, nameLength;
    Name name;
    if (!ReadName(buffer, size, offset, name, nameLength)) {
        return false;
    }

    if (offset + 0x004 > size) {
        return false;
    }
    u16 qtype = Bytes::ReadBE<u16>(buffer, offset + 0x000);
    u16 qclass = Bytes::ReadBE<u16>(buffer, offset + 0x002);
    if (qclass != 1) {
        return false;
    }
    offset += 0x004;

    switch (qtype) {
    case A:
        return readResponse(now, buffer, size, offset, id, ancount, m_aEntries);
    case SRV:
        return readResponse(now, buffer, size, offset, id, ancount, m_srvEntries);
    default:
        return false;
    }
}

bool DNS::hasResolver(const Address &resolver) const {
    for (u32 i = 0; i < m_resolvers.count(); i++) {
        if (m_resolvers[i] == resolver.address && 53 == resolver.port) {
            return true;
        }
    }
    return false;
}

bool DNS::readResponse(const u8 *buffer, u32 offset, AEntry &entry, u16 rdlength) {
    if (rdlength != 4) {
        return false;
    }

    entry.data = Bytes::ReadBE<u32>(buffer, offset + 0x000);
    return true;
}

bool DNS::readResponse(const u8 *buffer, u32 offset, SRVEntry &entry, u16 rdlength) {
    if (rdlength < 6) {
        return false;
    }

    u16 port = Bytes::ReadBE<u16>(buffer, offset + 0x004);
    offset += 0x006;

    Name name;
    u32 nameLength;
    if (!ReadName(buffer, offset - 6 + rdlength, offset, name, nameLength)) {
        return false;
    }

    entry.data = (Target){port, name};
    return true;
}

bool DNS::writeQuery(const Name &name, u16 qtype) {
    u8 buffer[512];
    u16 flags = 0;
    flags |= 1 << 8; // RD
    Bytes::WriteBE<u16>(buffer, 0x000, m_id);
    Bytes::WriteBE<u16>(buffer, 0x002, flags);
    Bytes::WriteBE<u16>(buffer, 0x004, 1); // QDCOUNT
    Bytes::WriteBE<u16>(buffer, 0x006, 0); // ANCOUNT
    Bytes::WriteBE<u16>(buffer, 0x008, 0); // NSCOUNT
    Bytes::WriteBE<u16>(buffer, 0x00a, 0); // ARCOUNT

    u32 offset = 0x00c, nameLength;
    if (!WriteName(buffer, Count(buffer), offset, name, nameLength)) {
        return false;
    }

    Bytes::WriteBE<u16>(buffer, offset + 0x000, qtype);
    Bytes::WriteBE<u16>(buffer, offset + 0x002, 1); // QCLASS

    for (u32 i = 0; i < m_resolvers.count(); i++) {
        Address resolver = {m_resolvers[i], 53};
        m_socket.sendTo(buffer, offset + 0x004, resolver);
    }
    return true;
}
