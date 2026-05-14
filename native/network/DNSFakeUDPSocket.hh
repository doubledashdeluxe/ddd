#pragma once

#include "native/network/FakeUDPSocket.hh"

#include <portable/network/DNS.hh>

#include <map>
#include <string>
#include <vector>

class DNSFakeUDPSocket final : public FakeUDPSocket {
public:
    struct Target {
        u16 port;
        std::string name;
    };

    using AEntries = std::map<std::string, u32>;
    using SRVEntries = std::map<std::string, Target>;

    DNSFakeUDPSocket(const AEntries &aEntries, const SRVEntries &srvEntries);
    ~DNSFakeUDPSocket();

    s32 sendTo(const void *buffer, u32 size, const Address &address) override;

private:
    struct Query {
        u16 id;
        DNS::Name name;
        u16 qtype;
    };

    struct Response {
        u16 id;
        DNS::Name name;
        u16 type;
        std::optional<std::vector<u8>> data;
    };

    std::optional<Query> readQuery(const u8 *buffer, u32 size);
    void writeResponse(const Response &response);

    std::vector<u8> m_data;
    const AEntries &m_aEntries;
    const SRVEntries &m_srvEntries;
};
