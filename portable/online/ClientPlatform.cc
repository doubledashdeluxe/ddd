#include "ClientPlatform.hh"

ClientPlatform::ClientPlatform(Allocator &allocator, Random &random, DNS &dns, UDPSocket &socket,
        const Array<u8, 32> &clientK)
    : allocator(allocator), random(random), dns(dns), socket(socket), clientK(clientK) {}
