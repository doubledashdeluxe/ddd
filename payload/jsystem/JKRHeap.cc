#include "JKRHeap.hh"

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
#else
void *operator new[](size_t size, JKRHeap * /* heap */, s32 /* alignment */) {
    return new u8[size];
}
#endif
