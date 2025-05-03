#pragma once

#include "portable/Allocator.hh"
#include "portable/Array.hh"
#include "portable/crypto/Random.hh"
#include "portable/network/DNS.hh"
#include "portable/network/Network.hh"
#include "portable/network/UDPSocket.hh"
#include "portable/online/ServerManager.hh"

struct ClientPlatform {
    ClientPlatform(Allocator &allocator, Random &random, Network &network, DNS &dns,
            UDPSocket &socket, ServerManager &serverManager, const Array<u8, 32> &clientK);

    Allocator &allocator;
    Random &random;
    Network &network;
    DNS &dns;
    UDPSocket &socket;
    ServerManager &serverManager;
    const Array<u8, 32> &clientK;
};
