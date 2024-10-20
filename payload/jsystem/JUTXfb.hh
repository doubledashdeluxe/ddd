#pragma once

#include "jsystem/JKRHeap.hh"

#include <payload/Replace.hh>

class JUTXfb {
public:
    void *getDrawingXfb() const;

    void swapBuffers();

    static JUTXfb *GetManager();

private:
    REPLACE void initiate(u16 width, u16 height, JKRHeap *heap, s32 count);

    void *m_buffers[3];
    bool m_validities[3];
    s32 m_count;
    s16 m_drawingXfbIndex;
    s16 m_drawnXfbIndex;
    u8 _18[0x20 - 0x18];

    static JUTXfb *s_manager;
};
size_assert(JUTXfb, 0x20);
