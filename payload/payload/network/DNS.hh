#pragma once

#include "payload/network/UDPSocket.hh"

#include <portable/Array.hh>
#include <portable/Ring.hh>

class DNS {
public:
    bool resolve(const char *name, u32 &address);

    static void Init();
    static DNS *Instance();

private:
    struct Query {
        s64 expirationTime;
        u16 id;
        Array<char, 256> name;
    };

    struct Response {
        s64 expirationTime;
        Array<char, 256> name;
        u32 address;
    };

    DNS();

    bool readResponse(Response &response);
    bool writeQuery(const Query &query);

    UDPSocket m_socket;
    Array<SOSockAddr, 2> m_resolvers;
    u16 m_id;
    Ring<Query, 32> m_queries;
    Ring<Response, 256> m_responses;

    static DNS *s_instance;
};
