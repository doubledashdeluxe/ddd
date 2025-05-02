#pragma once

#include <jsystem/JKRHeap.hh>
#include <portable/Allocator.hh>

class HeapAllocator : public Allocator {
public:
    HeapAllocator(JKRHeap *heap);

    void *alloc(size_t size) override;

private:
    JKRHeap *m_heap;
};
