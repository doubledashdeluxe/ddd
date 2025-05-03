#pragma once

#include "portable/Array.hh"
#include "portable/Ring.hh"
#include "portable/network/Address.hh"
#include "portable/network/UDPSocket.hh"

class DNS {
public:
    bool resolve(const char *name, u32 &address);

protected:
    DNS(UDPSocket &socket);
    ~DNS();

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

    virtual s64 secondsToTicks(s64 seconds) = 0;
    virtual s64 getMonotonicTicks() = 0;

    bool readResponse(Response &response);
    bool writeQuery(const Query &query);

    UDPSocket &m_socket;
    Array<Address, 2> m_resolvers;
    u16 m_id;
    Ring<Query, 32> m_queries;
    Ring<Response, 256> m_responses;
};
