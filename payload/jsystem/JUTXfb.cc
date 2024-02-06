#include "JUTXfb.hh"

void *JUTXfb::getDrawingXfb() const {
    return (m_drawingXfbIndex >= 0) ? m_buffers[m_drawingXfbIndex] : nullptr;
}

JUTXfb *JUTXfb::GetManager() {
    return s_manager;
}
