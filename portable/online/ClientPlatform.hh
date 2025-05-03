#pragma once

#include "portable/Allocator.hh"
#include "portable/Array.hh"
#include "portable/crypto/Random.hh"
#include "portable/network/DNS.hh"
#include "portable/network/UDPSocket.hh"

struct ClientPlatform {
    ClientPlatform(Allocator &allocator, Random &random, DNS &dns, UDPSocket &socket,
            const Array<u8, 32> &clientK);

    Allocator &allocator;
    Random &random;
    DNS &dns;
    UDPSocket &socket;
    const Array<u8, 32> &clientK;
};
