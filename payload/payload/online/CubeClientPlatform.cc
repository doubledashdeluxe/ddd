#include "CubeClientPlatform.hh"

#include "payload/crypto/CubeRandom.hh"
#include "payload/network/CubeDNS.hh"

CubeClientPlatform::CubeClientPlatform(JKRHeap *heap) : m_allocator(heap) {}

Allocator &CubeClientPlatform::allocator() {
    return m_allocator;
}

Random &CubeClientPlatform::random() {
    return *CubeRandom::Instance();
}

DNS &CubeClientPlatform::dns() {
    return *CubeDNS::Instance();
}
