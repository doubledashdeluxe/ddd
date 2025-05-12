#pragma once

#include "native/network/FakeUDPSocket.hh"

#include <map>
#include <string>
#include <vector>

class DNSFakeUDPSocket final : public FakeUDPSocket {
public:
    DNSFakeUDPSocket(const std::map<std::vector<std::string>, u32> &servers);
    ~DNSFakeUDPSocket();

    s32 sendTo(const void *buffer, u32 size, const Address &address) override;

private:
    struct Query {
        u16 id;
        std::vector<std::string> parts;
    };

    struct Response {
        u16 id;
        std::vector<std::string> parts;
        std::optional<u32> address;
    };

    std::optional<Query> readQuery(const u8 *buffer, u32 size);
    void writeResponse(const Response &response);

    std::vector<u8> m_data;
    const std::map<std::vector<std::string>, u32> &m_servers;
};
