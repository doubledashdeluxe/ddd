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
