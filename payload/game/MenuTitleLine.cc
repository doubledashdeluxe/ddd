#include "MenuTitleLine.hh"

void MenuTitleLine::drop(const char *title) {
    m_title = Title::Max;
    m_titlePictures[0]->changeTexture(title, 0);
    m_titlePictures[0]->m_isVisible = true;
    m_titlePictures[1]->m_isVisible = false;
    m_state = 1;
}

void MenuTitleLine::draw(const J2DGraphContext *graphContext) {
    m_screen.draw(0.0f, 0.0f, graphContext);
}

MenuTitleLine *MenuTitleLine::Create(JKRArchive *archive, JKRHeap *heap) {
    if (!s_instance) {
        s_instance = new (heap, 0x4) MenuTitleLine(archive);
    }
    return s_instance;
}

void MenuTitleLine::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

MenuTitleLine *MenuTitleLine::Instance() {
    return s_instance;
}

MenuTitleLine::~MenuTitleLine() {}
