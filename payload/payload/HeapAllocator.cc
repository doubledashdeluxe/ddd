#include "HeapAllocator.hh"

HeapAllocator::HeapAllocator(JKRHeap *heap) : m_heap(heap) {}

void *HeapAllocator::alloc(size_t size) {
    return operator new(size, m_heap, 0x4);
}
