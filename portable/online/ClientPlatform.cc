#include "ClientPlatform.hh"

ClientPlatform::ClientPlatform(Allocator &allocator, Random &random, DNS &dns,
        const Array<u8, 32> &clientK)
    : allocator(allocator), random(random), dns(dns), clientK(clientK) {}
