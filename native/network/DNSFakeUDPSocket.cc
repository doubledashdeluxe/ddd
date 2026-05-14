#include "DNSFakeUDPSocket.hh"

#include <portable/Bytes.hh>

#include <cassert>

DNSFakeUDPSocket::DNSFakeUDPSocket(const AEntries &aEntries, const SRVEntries &srvEntries)
    : FakeUDPSocket(m_data)
    , m_aEntries(aEntries)
    , m_srvEntries(srvEntries) {}

DNSFakeUDPSocket::~DNSFakeUDPSocket() = default;

s32 DNSFakeUDPSocket::sendTo(const void *buffer, u32 size, const Address &address) {
    auto query = readQuery(static_cast<const u8 *>(buffer), size);
    if (query) {
        std::string name(query->name.values());
        std::optional<std::vector<u8>> data;
        switch (query->qtype) {
        case DNS::A:
            if (auto entry = m_aEntries.find(name); entry != m_aEntries.end()) {
                data.emplace(4);
                Bytes::WriteBE<u32>(data->data(), 0x000, entry->second);
            }
            break;
        case DNS::SRV:
            if (auto entry = m_srvEntries.find(name); entry != m_srvEntries.end()) {
                data.emplace(6 + entry->second.name.size() + 2);
                Bytes::WriteBE<u16>(data->data(), 0x000, 0); // Priority
                Bytes::WriteBE<u16>(data->data(), 0x002, 0); // Weight
                Bytes::WriteBE<u16>(data->data(), 0x004, entry->second.port);
                u32 offset = 0x006, nameLength;
                DNS::Name name;
                snprintf(name.values(), name.count(), "%s", entry->second.name.c_str());
                assert(DNS::WriteName(data->data(), data->size(), offset, name, nameLength));
            }
            break;
        }
        Response response{query->id, query->name, query->qtype, data};
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

    std::vector<std::string> parts;
    u32 offset = 0x00c, nameLength;
    DNS::Name name;
    if (!DNS::ReadName(buffer, size, offset, name, nameLength)) {
        return {};
    }

    if (offset + 0x004 > size) {
        return {};
    }
    u16 qtype = Bytes::ReadBE<u16>(buffer, offset + 0x000);
    u16 qclass = Bytes::ReadBE<u16>(buffer, offset + 0x002);
    if (qclass != 1) {
        return {};
    }

    return Query{id, name, qtype};
}

void DNSFakeUDPSocket::writeResponse(const Response &response) {
    u32 size = 0;
    size += 0x00c;
    size += strlen(response.name.values()) + 2;
    size += 0x004;
    if (response.data) {
        size += 0x00c + response.data->size();
    }
    m_data.resize(size);

    u16 flags = 0;
    flags |= 1 << 15;                      // QR
    flags |= 1 << 8;                       // RD
    flags |= 1 << 7;                       // RA
    flags |= (response.data ? 0 : 3) << 0; // RCODE
    Bytes::WriteBE<u16>(m_data.data(), 0x000, response.id);
    Bytes::WriteBE<u16>(m_data.data(), 0x002, flags);
    Bytes::WriteBE<u16>(m_data.data(), 0x004, 1);               // QDCOUNT
    Bytes::WriteBE<u16>(m_data.data(), 0x006, !!response.data); // ANCOUNT
    Bytes::WriteBE<u16>(m_data.data(), 0x008, 0);               // NSCOUNT
    Bytes::WriteBE<u16>(m_data.data(), 0x00a, 0);               // ARCOUNT

    u32 offset = 0x00c, nameLength;
    DNS::Name name;
    assert(DNS::WriteName(m_data.data(), m_data.size(), offset, response.name, nameLength));

    Bytes::WriteBE<u16>(m_data.data(), offset + 0x000, response.type); // QTYPE
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x002, 1);             // QCLASS
    offset += 0x004;

    if (!response.data) {
        return;
    }

    const std::vector<u8> &data = *response.data;
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x000, 3 << 14 | 0x00c); // NAME
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x002, response.type);   // TYPE
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x004, 1);               // CLASS
    Bytes::WriteBE<u32>(m_data.data(), offset + 0x006, 3600);            // TTL
    Bytes::WriteBE<u16>(m_data.data(), offset + 0x00a, data.size());     // RDLENGTH
    memcpy(m_data.data() + offset + 0x00c, data.data(), data.size());    // RDATA
}
