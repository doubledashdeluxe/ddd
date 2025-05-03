#include "ClientPlatform.hh"

ClientPlatform::ClientPlatform(Allocator &allocator, Random &random, DNS &dns, UDPSocket &socket,
        ServerManager &serverManager, const Array<u8, 32> &clientK)
    : allocator(allocator), random(random), dns(dns), socket(socket), serverManager(serverManager),
      clientK(clientK) {}
