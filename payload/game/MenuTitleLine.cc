#include "MenuTitleLine.hh"

void MenuTitleLine::draw(const J2DGraphContext *graphContext) {
    m_screen.draw(0.0f, 0.0f, graphContext);
}

MenuTitleLine *MenuTitleLine::Create(JKRArchive *archive, JKRHeap *heap) {
    if (!s_instance) {
        s_instance = new (heap, 0x4) MenuTitleLine(archive);
    }
    return s_instance;
}

MenuTitleLine *MenuTitleLine::Instance() {
    return s_instance;
}
