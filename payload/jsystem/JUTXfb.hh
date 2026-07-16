#pragma once

#include "jsystem/JKRHeap.hh"

#include <payload/Replace.hh>

class JUTXfb {
public:
    class State {
    public:
        enum {
            Drawn = 0,
            Drawing = 1,
            Displaying = 2,
        };

    private:
        State();
    };

    static JUTXfb *Create(JKRHeap *heap, s32 count);
    static JUTXfb *Instance();

private:
    REPLACE void initiate(u16 width, u16 height, JKRHeap *heap, s32 count);

    void *m_buffers[3];
    bool m_validities[3];
    s32 m_count;

public:
    s16 m_drawingXfbIndex;
    s16 m_drawnXfbIndex;
    s16 m_displayingXfbIndex;
    s32 m_state;

private:
    static JUTXfb *s_instance;
};
size_assert(JUTXfb, 0x20);
