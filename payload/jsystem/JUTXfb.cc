#include "JUTXfb.hh"

#include <common/VI.hh>

extern "C" {
#include <assert.h>
}

void *JUTXfb::getDrawingXfb() const {
    return (m_drawingXfbIndex >= 0) ? m_buffers[m_drawingXfbIndex] : nullptr;
}

JUTXfb *JUTXfb::GetManager() {
    return s_manager;
}

void JUTXfb::initiate(u16 width, u16 height, JKRHeap *heap, s32 count) {
    VI *vi = VI::Instance();
    assert(width == vi->getXFBWidth());
    assert(height == vi->getXFBHeight());
    assert(heap);
    assert(count == 2);
    m_buffers[0] = vi->getXFB();
    m_buffers[1] = new (heap, 0x20) u8[vi->getXFBSize()];
    m_buffers[2] = nullptr;
    for (u32 i = 0; i < 3; i++) {
        m_validities[i] = m_buffers[i];
    }
}
