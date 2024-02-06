#pragma once

#include <common/Types.hh>

class JUTXfb {
public:
    void *getDrawingXfb() const;

    static JUTXfb *GetManager();

private:
    void *m_buffers[3];
    u8 _0c[0x14 - 0x0c];
    s16 m_drawingXfbIndex;
    u8 _16[0x20 - 0x16];

    static JUTXfb *s_manager;
};
size_assert(JUTXfb, 0x20);
