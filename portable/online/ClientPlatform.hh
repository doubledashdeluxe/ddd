#pragma once

#include "portable/Allocator.hh"
#include "portable/crypto/Random.hh"
#include "portable/crypto/Types.hh"
#include "portable/network/DNS.hh"
#include "portable/network/Network.hh"
#include "portable/network/UDPSocket.hh"
#include "portable/online/ServerManager.hh"

struct ClientPlatform {
    ClientPlatform(Allocator &allocator, Random &random, Network &network, DNS &dns,
            UDPSocket &socket, ServerManager &serverManager, const Key &clientK);

    Allocator &allocator;
    Random &random;
    Network &network;
    DNS &dns;
    UDPSocket &socket;
    ServerManager &serverManager;
    const Key &clientK;
};
