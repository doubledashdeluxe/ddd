#include "ClientPlatform.hh"

ClientPlatform::ClientPlatform(Allocator &allocator, Random &random, Network &network, DNS &dns,
        UDPSocket &socket, ServerManager &serverManager, const Array<u8, 32> &clientK)
    : allocator(allocator), random(random), network(network), dns(dns), socket(socket),
      serverManager(serverManager), clientK(clientK) {}
