#include "JKRExpHeap.hh"

u32 JKRExpHeap::MemBlock::getSize() const {
    return m_size;
}

const JKRExpHeap::MemBlock *JKRExpHeap::MemBlock::getPrev() const {
    return m_prev;
}

const JKRExpHeap::MemBlock *JKRExpHeap::MemBlock::getNext() const {
    return m_next;
}

const JKRExpHeap::MemBlock *JKRExpHeap::getFreeListHead() const {
    return m_freeListHead;
}
