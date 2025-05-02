#include "CubeClientPlatform.hh"

CubeClientPlatform::CubeClientPlatform(JKRHeap *heap) : m_allocator(heap) {}

Allocator &CubeClientPlatform::allocator() {
    return m_allocator;
}
