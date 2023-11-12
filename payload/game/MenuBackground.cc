#include "MenuBackground.hh"

void MenuBackground::draw(const J2DGraphContext *graphContext) {
    m_screen.draw(0.0f, 0.0f, graphContext);
}

MenuBackground *MenuBackground::Create(JKRArchive *archive) {
    if (!s_instance) {
        s_instance = new MenuBackground(archive);
    }
    return s_instance;
}

MenuBackground *MenuBackground::Instance() {
    return s_instance;
}
