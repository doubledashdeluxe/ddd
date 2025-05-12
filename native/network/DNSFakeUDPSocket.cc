#include "DNSFakeUDPSocket.hh"

#include <portable/Bytes.hh>

#include <cstring>

DNSFakeUDPSocket::DNSFakeUDPSocket(const std::map<std::vector<std::string>, u32> &servers)
    : FakeUDPSocket(m_data), m_servers(servers) {}

DNSFakeUDPSocket::~DNSFakeUDPSocket() = default;

s32 DNSFakeUDPSocket::sendTo(const void *buffer, u32 size, const Address &address) {
    auto query = readQuery(static_cast<const u8 *>(buffer), size);
    if (query) {
        std::optional<u32> address;
        if (auto server = m_servers.find(query->parts); server != m_servers.end()) {
            address = server->second;
        }
        Response response{query->id, query->parts, address};
        writeResponse(response);
    }

    return FakeUDPSocket::sendTo(buffer, size, address);
}

std::optional<DNSFakeUDPSocket::Query> DNSFakeUDPSocket::readQuery(const u8 *buffer, u32 size) {
    if (size < 0x00c) {
        return {};
    }

    u16 id = Bytes::ReadBE<u16>(buffer, 0x000);
    u16 flags = 0;
    flags |= 1 << 8; // RD
    if (Bytes::ReadBE<u16>(buffer, 0x002) != flags) {
        return {};
    }

    u16 qdcount = Bytes::ReadBE<u16>(buffer, 0x004);
    u16 ancount = Bytes::ReadBE<u16>(buffer, 0x006);
    u16 nscount = Bytes::ReadBE<u16>(buffer, 0x008);
    u16 arcount = Bytes::ReadBE<u16>(buffer, 0x00a);
    if (qdcount != 1 || ancount != 0 || nscount != 0 || arcount != 0) {
        return {};
    }
    if (size < 0x00c + qdcount * (0x002 + 0x004)) {
        return {};
    }

    std::vector<std::string> parts;
    u32 offset = 0x00c;
    while (true) {
        if (offset + 0x004 >= size) {
            return {};
        }

        u8 partLength = buffer[offset++];
        if (partLength >= 64) {
            return {};
        }

        if (partLength == 0) {
            break;
        }

        if (offset + partLength + 0x004 >= size) {
            return {};
        }

        auto &part = parts.emplace_back();
        part.resize(partLength);
        memcpy(part.data(), buffer + offset, partLength);
        offset += partLength;
    }

    u16 qtype = Bytes::ReadBE<u16>(buffer, offset + 0x000);
    u16 qclass = Bytes::ReadBE<u16>(buffer, offset + 0x002);
    if (qtype != 1 || qclass != 1) {
        return {};
    }

    return Query{id, parts};
}

void DNSFakeUDPSocket::writeResponse(const Response &response) {
    u32 nameLength = 0;
    for (auto &part : response.parts) {
        nameLength++;
        nameLength += part.length();
    }
    nameLength++;
    m_data.resize(0x00c + nameLength + 0x004 + response.address.has_value() * 0x010);

    u16 flags = 0;
    flags |= 1 << 15;                         // QR
    flags |= 1 << 8;                          // RD
    flags |= 1 << 7;                          // RA
    flags |= (response.address ? 0 : 3) << 0; // RCODE
    Bytes::WriteBE<u16>(m_data.data(), 0x000, response.id);
    Bytes::WriteBE<u16>(m_data.data(), 0x002, flags);
    Bytes::WriteBE<u16>(m_data.data(), 0x004, 1); // QDCOUNT
    Bytes::WriteBE<u16>(m_data.data(), 0x006, 1); // ANCOUNT
    Bytes::WriteBE<u16>(m_data.data(), 0x008, 0); // NSCOUNT
    Bytes::WriteBE<u16>(m_data.data(), 0x00a, 0); // ARCOUNT

    u32 offset = 0x00c;
    for (auto &part : response.parts) {
        m_data[offset++] = part.length();
        memcpy(m_data.data() + offset, part.data(), part.length());
        offset += part.length();
    }
    m_data[offset++] = 0;

    Bytes::WriteBE<u16>(m_data.data(), offset + 0x000, 1); // QTYPE
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x002, 1); // QCLASS
    offset += 0x004;

    if (!response.address) {
        return;
    }

    Bytes::WriteBE<u16>(m_data.data(), offset + 0x000, 3 << 14 | 0x00c);   // NAME
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x002, 1);                 // TYPE
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x004, 1);                 // CLASS
    Bytes::WriteBE<u32>(m_data.data(), offset + 0x006, 3600);              // TTL
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x00a, 4);                 // RDLENGTH
    Bytes::WriteBE<u32>(m_data.data(), offset + 0x00c, *response.address); // RDATA
}
