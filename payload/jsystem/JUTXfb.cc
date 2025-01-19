#include "JUTXfb.hh"

#include <common/VI.hh>

extern "C" {
#include <assert.h>
}

void JUTXfb::initiate(u16 width, u16 height, JKRHeap *heap, s32 /* count */) {
    m_count = 1;
    VI *vi = VI::Instance();
    assert(width == vi->getXFBWidth());
    assert(height == vi->getXFBHeight());
    assert(heap);
    m_buffers[0] = vi->getXFB();
    m_buffers[1] = nullptr;
    m_buffers[2] = nullptr;
    for (u32 i = 0; i < 3; i++) {
        m_validities[i] = m_buffers[i];
    }
}
