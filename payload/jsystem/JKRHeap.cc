#include "JKRHeap.hh"

void *JKRHeap::getStartAddr() const {
    return m_startAddr;
}

void *JKRHeap::getEndAddr() const {
    return m_endAddr;
}

JKRHeap *JKRHeap::GetSystemHeap() {
    return s_systemHeap;
}

JKRHeap *JKRHeap::GetCurrentHeap() {
    return s_currentHeap;
}

JKRHeap *JKRHeap::GetRootHeap() {
    return s_rootHeap;
}

#ifdef __CWCC__
void *operator new[](size_t size, JKRHeap *heap, s32 alignment) {
    if (!heap) {
        heap = JKRHeap::GetCurrentHeap();

        if (!heap) {
            return nullptr;
        }
    }

    return heap->alloc(size, alignment);
}
#endif
