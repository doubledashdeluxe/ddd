#include "CubeClientPlatform.hh"

#include "payload/network/CubeDNS.hh"

CubeClientPlatform::CubeClientPlatform(JKRHeap *heap) : m_allocator(heap) {}

Allocator &CubeClientPlatform::allocator() {
    return m_allocator;
}

DNS &CubeClientPlatform::dns() {
    return *CubeDNS::Instance();
}
